#include "core/client/ai/endless/endless.h"

#include <algorithm>

#include "core/frame_request.h"
#include "core/field_pretty_printer.h"

using namespace std;

Endless::Endless(unique_ptr<AI> ai) : ai_(std::move(ai))
{
}

EndlessResult Endless::run(const KumipuyoSeq& seq)
{
    // Initialize ai.
    FrameRequest req;
    req.frameId = 1;
    ai_->gameWillBegin(req);

    req.playerFrameRequest[0].kumipuyoSeq = seq;
    req.playerFrameRequest[1].kumipuyoSeq = seq;

    setEnemyField(&req);

    vector<Decision> decisions;

    int maxRensaScore = 0;
    int maxRensa = 0;
    for (int i = 0; i < 50; ++i) {
        req.frameId = i + 2;

        // For gazing.
        ai_->gaze(req.frameId, CoreField(req.enemyPlayerFrameRequest().field), req.enemyPlayerFrameRequest().kumipuyoSeq);

        // think
        ai_->next2Appeared(req);
        ai_->decisionRequested(req);

        AdditionalThoughtInfo info { ai_->myPlayerState(), ai_->enemyPlayerState() };
        DropDecision dropDecision = ai_->think(req.frameId, req.myPlayerFrameRequest().field,
                                               req.myPlayerFrameRequest().kumipuyoSeq, info, false);

        CoreField f(req.myPlayerFrameRequest().field);
        if (!f.dropKumipuyo(dropDecision.decision(), req.myPlayerFrameRequest().kumipuyoSeq.front())) {
            // couldn't drop. break.
            break;
        }

        if (verbose_) {
            FieldPrettyPrinter::print(f, req.playerFrameRequest[0].kumipuyoSeq.subsequence(1));
        }

        decisions.push_back(dropDecision.decision());

        RensaResult rensaResult = f.simulate();
        maxRensaScore = std::max(maxRensaScore, rensaResult.score);
        maxRensa = std::max(maxRensa, rensaResult.chains);
        if (f.color(3, 12) != PuyoColor::EMPTY) {
            return EndlessResult {
                .hand = i,
                .score = -1,
                .maxRensa = -1,
                .zenkeshi = false,
                .decisions = decisions,
            };
        }
        if (rensaResult.score > 10000) {
            // The main rensa must be fired.
            return EndlessResult {
                .hand = i,
                .score = rensaResult.score,
                .maxRensa = rensaResult.chains,
                .zenkeshi = f.isZenkeshi(),
                .decisions = decisions,
            };
        }
        if (f.isZenkeshi()) {
            return EndlessResult {
                .hand = i,
                .score = rensaResult.score,
                .maxRensa = rensaResult.chains,
                .zenkeshi = true,
                .decisions = decisions,
            };
        }
        req.playerFrameRequest[0].field = f;
        req.playerFrameRequest[0].kumipuyoSeq.dropFront();
        req.playerFrameRequest[1].kumipuyoSeq.dropFront();

        ai_->grounded(req);
    }

    return EndlessResult {
        .hand = 50,
        .score = maxRensaScore,
        .maxRensa = maxRensa,
        .zenkeshi = false,
        .decisions = decisions,
    };
}

void Endless::setEnemyField(FrameRequest* req)
{
    req->playerFrameRequest[1].field = PlainField(
            "Y   GY"
            "R   GG"
            "YRYGRY"
            "RYGRYY"
            "YRYGRG"
            "YRYGRG"
            "YGRYGR"
            "RYGRYG"
            "RYGRYG"
            "RYGRYG");
}
