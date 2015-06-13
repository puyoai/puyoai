#include "wii/wii_connect_server.h"

#include <iostream>
#include <vector>

#include "base/time.h"
#include "capture/analyzer.h"
#include "capture/source.h"
#include "core/core_field.h"
#include "core/game_result.h"
#include "core/frame_response.h"
#include "core/game_result.h"
#include "core/player.h"
#include "core/puyo_color.h"
#include "core/puyo_controller.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager_posix.h"
#include "core/server/game_state.h"
#include "core/server/game_state_observer.h"
#include "gui/screen.h"
#include "wii/key_sender.h"

using namespace std;

WiiConnectServer::WiiConnectServer(Source* source, Analyzer* analyzer,
                                   KeySender* p1KeySender, KeySender* p2KeySender,
                                   const string& p1Program, const string& p2Program) :
    shouldStop_(false),
    surface_(emptyUniqueSDLSurface()),
    source_(source),
    analyzer_(analyzer),
    keySenders_ { p1KeySender, p2KeySender }
{
    isAi_[0] = (p1Program != "-");
    isAi_[1] = (p2Program != "-");

    connector_.reset(new ConnectorManagerPosix {
        Connector::create(0, p1Program),
        Connector::create(1, p2Program),
    });
    connector_->setWaitTimeout(false);
}

WiiConnectServer::~WiiConnectServer()
{
    if (th_.joinable())
        th_.join();
}

void WiiConnectServer::addObserver(GameStateObserver* observer)
{
    CHECK(observer) << "observer should not be nullptr.";
    observers_.push_back(observer);
}

bool WiiConnectServer::start()
{
    th_ = thread([this]() {
        this->runLoop();
    });
    return true;
}

void WiiConnectServer::stop()
{
    shouldStop_ = true;
    if (th_.joinable())
        th_.join();
}

void WiiConnectServer::reset()
{
    for (int i = 0; i < 2; ++i) {
        lastDecision_[i] = Decision();
        message_[i].clear();
    }

    colorMap_.clear();
    colorMap_.insert(make_pair(RealColor::RC_EMPTY, PuyoColor::EMPTY));
    colorMap_.insert(make_pair(RealColor::RC_OJAMA, PuyoColor::OJAMA));
    colorsUsed_.fill(false);
}

void WiiConnectServer::runLoop()
{
    reset();

    bool gameStarted = false;
    int noSurfaceCount = 0;
    int frameId = 0;
    UniqueSDLSurface prevSurface(emptyUniqueSDLSurface());
    UniqueSDLSurface prev2Surface(emptyUniqueSDLSurface());

    while (!shouldStop_) {
        UniqueSDLSurface surface(source_->getNextFrame());
        if (!surface.get()) {
            ++noSurfaceCount;
            LOG(INFO) << "No surface?: count=" << noSurfaceCount << endl;
            cout << "No surface? count=" << noSurfaceCount << endl;
            // TODO(mayah): Why not sleep?
            if (noSurfaceCount > 100000) {
                shouldStop_ = true;
                break;
            }
            continue;
        }

        unique_ptr<AnalyzerResult> r = analyzer_->analyze(surface.get(), prevSurface.get(), prev2Surface.get(), analyzerResults_);
        LOG(INFO) << r->toString();

        switch (r->state()) {
        case CaptureGameState::UNKNOWN:
            if (!playForUnknown(frameId))
                shouldStop_ = true;
            break;
        case CaptureGameState::LEVEL_SELECT:
            // TODO(mayah): For workaround, we make frameId = 1.
            // Server should send some event to initialize a game state.
            // Client should implement an initialization logic
            if (!gameStarted) {
                // TODO(mayah): initialization should be done after NEXT1/NEXT2 are stabilized?
                lock_guard<mutex> lock(mu_);

                // If the first surface is level select, analyzerResults_ might be empty.
                // So, we need to allow analyzerResults_.empty() here.
                if (analyzerResults_.empty() || analyzerResults_.front()->state() != CaptureGameState::LEVEL_SELECT) {
                    cout << "New game started" << endl;
                    frameId = 1;
                    reset();
                    // The result might contain the previous game's result. We don't want to stabilize the result
                    // with using the previous game's results.
                    // So, remove all the results.
                    analyzerResults_.clear();
                    r->clear();

                    for (auto observer : observers_) {
                        observer->newGameWillStart();
                    }
                    gameStarted = true;
                }
            }
            if (!playForLevelSelect(frameId, *r))
                shouldStop_ = true;
            break;
        case CaptureGameState::PLAYING: {
            if (!playForPlaying(frameId, *r))
                shouldStop_ = true;
            GameState gameState = toGameState(frameId, *r);
            for (auto observer : observers_)
                observer->onUpdate(gameState);
            break;
        }
        case CaptureGameState::MATCH_FINISHED_WITH_DRAW:
        case CaptureGameState::MATCH_FINISHED_WITH_1P_WIN:
        case CaptureGameState::MATCH_FINISHED_WITH_2P_WIN:
        case CaptureGameState::GAME_FINISHED_WITH_DRAW:
        case CaptureGameState::GAME_FINISHED_WITH_1P_WIN:
        case CaptureGameState::GAME_FINISHED_WITH_2P_WIN:
            cout << "game finished detected: started?=" << gameStarted << endl;
            if (!playForFinished(frameId, gameStarted, *r))
                shouldStop_ = true;
            if (gameStarted) {
                GameResult gameResult = GameResult::DRAW;
                if (r->state() == CaptureGameState::GAME_FINISHED_WITH_1P_WIN || r->state() == CaptureGameState::MATCH_FINISHED_WITH_1P_WIN)
                    gameResult = GameResult::P1_WIN;
                if (r->state() == CaptureGameState::GAME_FINISHED_WITH_2P_WIN || r->state() == CaptureGameState::MATCH_FINISHED_WITH_2P_WIN)
                    gameResult = GameResult::P2_WIN;
                for (auto observer : observers_) {
                    // TODO(mayah): This is not DRAW, of course.
                    observer->gameHasDone(gameResult);
                }
                gameStarted = false;
            }
            break;
        }

        // We set frameId to surface's userdata. This will be useful for saving screen shot.
        surface->userdata = reinterpret_cast<void*>(static_cast<uintptr_t>(frameId));

        {
            lock_guard<mutex> lock(mu_);
            prev2Surface = move(prevSurface);
            prevSurface = move(surface_);
            surface_ = move(surface);
            analyzerResults_.push_front(move(r));
            while (analyzerResults_.size() > 10)
                analyzerResults_.pop_back();
        }

        frameId++;
    }
}

bool WiiConnectServer::playForUnknown(int frameId)
{
    if (frameId % 10 == 0) {
        for (int pi = 0; pi < 2; ++pi)
            keySenders_[pi]->sendKeySet(KeySet());
    }

    reset();
    return true;
}

bool WiiConnectServer::playForLevelSelect(int frameId, const AnalyzerResult& analyzerResult)
{
    if (frameId % 10 == 0) {
        for (int pi = 0; pi < 2; ++pi) {
            keySenders_[pi]->sendKeySet(KeySet());
            keySenders_[pi]->sendKeySet(KeySet(Key::RIGHT_TURN));
            keySenders_[pi]->sendKeySet(KeySet());
        }
    }

    // Sends an initialization message.
    for (int pi = 0; pi < 2; pi++) {
        if (connector_->connector(pi)->isClosed()) {
            LOG(INFO) << playerText(pi) << " disconnected";
            fprintf(stderr, "player #%d was disconnected\n", pi);
            return false;
        }

        if (isAi_[pi])
            connector_->connector(pi)->send(makeFrameRequestFor(pi, frameId, analyzerResult));
    }

    vector<FrameResponse> responses[2];
    connector_->receive(frameId, responses);

    return true;
}

bool WiiConnectServer::playForPlaying(int frameId, const AnalyzerResult& analyzerResult)
{
    double beginTime = currentTime();

    // Send KeySet() after detecting ojama-drop or grounded.
    // It's important that it is sent before requesting the decision to client,
    // because client may take time to return the rensponse.
    // Otherwise, puyo might be dropped for a few frames.
    for (int pi = 0; pi < 2; ++pi) {
        if (!isAi_[pi])
            continue;

        if (analyzerResult.playerResult(pi)->userEvent.ojamaDropped ||
            analyzerResult.playerResult(pi)->userEvent.grounded) {
            keySenders_[pi]->sendKeySet(KeySet());
        }
    }

    for (int pi = 0; pi < 2; pi++) {
        if (connector_->connector(pi)->isClosed()) {
            LOG(INFO) << playerText(pi) << " disconnected";
            fprintf(stderr, "player #%d was disconnected\n", pi);
            return false;
        }

        if (isAi_[pi])
            connector_->connector(pi)->send(makeFrameRequestFor(pi, frameId, analyzerResult));
    }

    vector<FrameResponse> responses[2];
    connector_->receive(frameId, responses);

    for (int pi = 0; pi < 2; pi++) {
        if (!isAi_[pi])
            continue;

        if (analyzerResult.playerResult(pi)->userEvent.grounded) {
            lastDecision_[pi] = Decision();
        }

        for (int j = static_cast<int>(responses[pi].size()) - 1; j >= 0; --j) {
            const FrameResponse& fr = responses[pi][j];
            if (!fr.message.empty()) {
                message_[pi] = fr.message;
                break;
            }
        }

        outputKeys(pi, analyzerResult, responses[pi], beginTime);
    }

    return true;
}

bool WiiConnectServer::playForFinished(int frameId, bool needsSendGameResult, const AnalyzerResult& analyzerResult)
{
    if (needsSendGameResult) {
        for (int pi = 0; pi < 2; ++pi) {
            if (!isAi_[pi])
                continue;
            connector_->connector(pi)->send(makeFrameRequestFor(pi, frameId, analyzerResult));
        }

        vector<FrameResponse> responses[2];
        connector_->receive(frameId, responses);
    }

    if (frameId % 10 == 0) {
        for (int pi = 0; pi < 2; ++pi) {
            keySenders_[pi]->sendKeySet(KeySet());
            keySenders_[pi]->sendKeySet(KeySet(Key::START));
            keySenders_[pi]->sendKeySet(KeySet());
        }
    }

    reset();
    return true;
}

FrameRequest WiiConnectServer::makeFrameRequestFor(int playerId, int frameId, const AnalyzerResult& re)
{
    FrameRequest fr;
    fr.frameId = frameId;
    switch (re.state()) {
    case CaptureGameState::UNKNOWN:
    case CaptureGameState::LEVEL_SELECT:
    case CaptureGameState::PLAYING:
        fr.gameResult = GameResult::PLAYING;
        break;
    case CaptureGameState::MATCH_FINISHED_WITH_1P_WIN:
    case CaptureGameState::GAME_FINISHED_WITH_1P_WIN:
        fr.gameResult = (playerId == 0 ? GameResult::P1_WIN : GameResult::P2_WIN);
        break;
    case CaptureGameState::MATCH_FINISHED_WITH_2P_WIN:
    case CaptureGameState::GAME_FINISHED_WITH_2P_WIN:
        fr.gameResult = (playerId == 0 ? GameResult::P2_WIN : GameResult::P1_WIN);
        break;
    case CaptureGameState::MATCH_FINISHED_WITH_DRAW:
    case CaptureGameState::GAME_FINISHED_WITH_DRAW:
        fr.gameResult = GameResult::DRAW;
        break;
    }

    for (int i = 0; i < 2; ++i) {
        int pi = playerId == 0 ? i : (1 - i);
        PlayerFrameRequest& pfr = fr.playerFrameRequest[i];
        const PlayerAnalyzerResult* pr = re.playerResult(pi);
        if (!pr)
            continue;

        pfr.kumipuyoSeq = KumipuyoSeq {
            Kumipuyo(toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS), true),
                     toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD), true)),
            Kumipuyo(toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS), true),
                     toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD), true)),
            Kumipuyo(toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS), true),
                     toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD), true)),
        };

        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                pfr.field.setColor(x, y, toPuyoColor(pr->adjustedField.field.get(x, y)));
            }
        }

        pfr.event = pr->userEvent;

        // We cannot detect correct values, so we use some default values.
        pfr.kumipuyoPos = KumipuyoPos(3, 12, 0);
        pfr.score = 0;
        pfr.ojama = 0;
    }

    if (re.state() == CaptureGameState::MATCH_FINISHED_WITH_1P_WIN ||
        re.state() == CaptureGameState::MATCH_FINISHED_WITH_2P_WIN ||
        re.state() == CaptureGameState::MATCH_FINISHED_WITH_DRAW) {
        fr.matchEnd = true;
        cout << "match end" << endl;
    }

    return fr;
}

PuyoColor WiiConnectServer::toPuyoColor(RealColor rc, bool allowAllocation)
{
    auto it = colorMap_.find(rc);
    if (it != colorMap_.end())
        return it->second;

    // RC_EMPTY is always mapped to EMPTY.
    if (rc == RealColor::RC_EMPTY)
        return PuyoColor::EMPTY;

    if (!allowAllocation) {
        LOG(WARNING) << toString(rc) << " cannot mapped to be a puyo color. EMPTY is returned instead.";
        return PuyoColor::EMPTY;
    }

    DCHECK(isNormalColor(rc)) << toString(rc);
    for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
        if (colorsUsed_[i])
            continue;

        colorsUsed_[i] = true;
        PuyoColor pc = NORMAL_PUYO_COLORS[i];
        colorMap_.insert(make_pair(rc, pc));

        cout << "Detected a new color: " << toString(rc) << "->" << toString(pc) << endl;
        LOG(INFO) << "Detected a new color: " << toString(rc) << "->" << toString(pc);
        return pc;
    }

    // 5th color comes... :-(
    LOG(ERROR) << "Detected 5th color: " << toString(rc);
    return PuyoColor::EMPTY;
}

GameState WiiConnectServer::toGameState(int frameId, const AnalyzerResult& analyzerResult)
{
    GameState gameState(frameId);
    for (int pi = 0; pi < 2; ++pi) {
        PlayerGameState* pgs = gameState.mutablePlayerGameState(pi);
        const PlayerAnalyzerResult* pr = analyzerResult.playerResult(pi);

        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                pgs->field.setColor(x, y, toPuyoColor(pr->adjustedField.field.get(x, y)));
            }
        }

        PuyoColor pcs[6] = {
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::CURRENT_AXIS)),
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::CURRENT_CHILD)),
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT1_AXIS)),
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT1_CHILD)),
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT2_AXIS)),
            toPuyoColor(pr->adjustedField.realColor(NextPuyoPosition::NEXT2_CHILD)),
        };

        KumipuyoSeq seq;
        for (int i = 0; i < 3; ++i) {
            if (pcs[2 * i] == PuyoColor::EMPTY || pcs[2 * i + 1] == PuyoColor::EMPTY)
                break;
            seq.add(Kumipuyo(pcs[2 * i], pcs[2 * i + 1]));
        }
        pgs->kumipuyoSeq = seq;

        pgs->event = pr->userEvent;
        pgs->dead = false;
        pgs->playable = pr->playable;
        pgs->score = 0;
        pgs->pendingOjama = 0;
        pgs->fixedOjama = 0;
        if (pr->playable) {
            pgs->decision = lastDecision_[pi];
            pgs->kumipuyoPos = calculateDropPosition(pgs->field, lastDecision_[pi]);
        } else {
            pgs->decision = Decision();
            pgs->kumipuyoPos = KumipuyoPos();
        }
        pgs->message = message_[pi];
    }

    return gameState;
}

void WiiConnectServer::outputKeys(int pi, const AnalyzerResult& analyzerResult,
                                  const vector<FrameResponse>& responses, double beginTime)
{
    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = responses.size(); i > 0; ) {
        i--;

        const Decision& d = responses[i].decision;

        if (!d.isValid() || d == lastDecision_[pi])
            continue;

        // ----------
        // Set's the current area.
        // This is for checking the destination is reachable, it is ok to set ojama if the cell is occupied.
        KeySetSeq keySetSeq;
        {
            CoreField field;
            const AdjustedField& af = analyzerResult.playerResult(pi)->adjustedField;
            for (int x = 1; x <= 6; ++x) {
                for (int y = 1; y <= 12; ++y) {
                    if (af.field.get(x, y) == RealColor::RC_EMPTY)
                        break;

                    field.dropPuyoOn(x, PuyoColor::OJAMA);
                }
            }

            keySetSeq = PuyoController::findKeyStroke(field, d).seq();
            if (keySetSeq.empty()) {
                cout << "Cannot move?" << endl;
                continue;
            }
        }

        lastDecision_[pi] = d;
        if (!keySetSeq.empty()) {
            double endTime = currentTime();
            cout << (endTime - beginTime) << endl;
            double duration = 33.3333 - (endTime - beginTime) * 1000;
            cout << duration << endl;

            // TODO(mayah): Maybe we need to wait using duration?
            keySenders_[pi]->sendWait(20);
        }

        keySenders_[pi]->sendKeySetSeq(keySetSeq);
        return;
    }
}

void WiiConnectServer::draw(Screen* screen)
{
    SDL_Surface* surface = screen->surface();
    if (!surface)
        return;

    lock_guard<mutex> lock(mu_);

    if (!surface_.get())
        return;

    surface->userdata = surface_->userdata;
    SDL_Rect rect = screen->mainBox().toSDLRect();
    SDL_BlitSurface(surface_.get(), nullptr, surface, &rect);
}

unique_ptr<AnalyzerResult> WiiConnectServer::analyzerResult() const
{
    lock_guard<mutex> lock(mu_);

    if (analyzerResults_.empty())
        return unique_ptr<AnalyzerResult>();

    return analyzerResults_.front().get()->copy();
}

// static
KumipuyoPos WiiConnectServer::calculateDropPosition(const PlainField& pf, const Decision& decision)
{
    int x1 = decision.axisX();
    int x2 = decision.childX();

    if (x1 == x2) {
        for (int y = 1; y <= 13; ++y) {
            if (pf.color(x1, y) == PuyoColor::EMPTY) {
                if (decision.rot() == 0) {
                    return KumipuyoPos(x1, y, 0);
                } else {
                    return KumipuyoPos(x1, y + 1, 2);
                }
            }
        }

        // No position to drop?
        return KumipuyoPos();
    }

    int h1 = 0;
    for (int y = 1; y <= 14; ++y) {
        if (pf.isEmpty(x1, y)) {
            h1 = y;
            break;
        }
    }
    int h2 = 0;
    for (int y = 1; y <= 14; ++y) {
        if (pf.isEmpty(x2, y)) {
            h2 = y;
            break;
        }
    }

    if (h1 == 0 || h2 == 0)
        return KumipuyoPos();

    int y = std::max(h1, h2);
    return KumipuyoPos(x1, y, decision.rot());
}
