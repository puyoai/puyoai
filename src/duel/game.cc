#include "game.h"

#include <cstdlib>
#include <iostream>
#include <glog/logging.h>
#include <sstream>
#include <string>
#include <vector>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/sequence_generator.h"
#include "core/state.h"
#include "duel/duel_server.h"
#include "duel/field_realtime.h"
#include "duel/frame_context.h"
#include "duel/game_state.h"

using namespace std;

Game::Game(DuelServer* duelServer) :
    duelServer_(duelServer)
{
    KumipuyoSeq seq = generateSequence();
    LOG(INFO) << "Puyo sequence=" << seq.toString();

    for (int i = 0; i < 2; i++) {
        field[i].reset(new FieldRealtime(i, seq));
        latest_decision_[i] = Decision::NoInputDecision();
    }
}

Game::~Game() {}

namespace {

/**
 * Updates decision when an applicable one is found.
 * Returns:
 *   if there is an accepted decision:
 *     its index in the given data array.
 *   else:
 *     -1
 */
int UpdateDecision(const vector<ReceivedData>& data, const FieldRealtime& field,
                   Decision* decision)
{
    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = data.size(); i > 0;) {
        i--;

        Decision d = data[i].decision;

        // We don't send ACK/NACK for invalid decision.
        if (!d.isValid())
            continue;

        Key key = field.GetKey(d);
        if (key != KEY_NONE) {
            *decision = d;
            return i;
        }
    }

    return -1;
}

} // namespace

void Game::Play(const vector<ReceivedData> data[2])
{
    for (int pi = 0; pi < 2; pi++) {
        FieldRealtime* me = field[pi].get();
        string accepted_message;
        Key key = KEY_NONE;

        int accepted_index = UpdateDecision(data[pi], *field[pi], &latest_decision_[pi]);

        // Take care of ack_info.
        ack_info_[pi] = vector<int>(data[pi].size(), 0);
        for (size_t j = 0; j < data[pi].size(); j++) {
            const ReceivedData& d = data[pi][j];

            // This case does not require ack.
            if (!d.isValid())
                continue;

            if (static_cast<int>(j) == accepted_index) {
                ack_info_[pi][j] = d.frameId;
            } else {
                ack_info_[pi][j] = -d.frameId; // TODO(mayah): Negative means NACK. Weird.
            }
        }

        if (accepted_index != -1) {
            accepted_message = data[pi][accepted_index].msg;
        }

        key = me->GetKey(latest_decision_[pi]);

        FrameContext context;
        me->PlayOneFrame(key, &context);

        FieldRealtime* opponent = field[0].get() == me ? field[1].get() : field[0].get();
        context.apply(me, opponent);

        // Clear current key input if the move is done.
        if (me->userState().grounded) {
            latest_decision_[pi] = Decision::NoInputDecision();
        }

        if (accepted_message != "") {
            last_accepted_messages_[pi] = accepted_message;
        }
    }

    // TODO(mayah): Play should return GameState, and DuelServer updates its state
    // in runLoop(). If we do so, Game does not need duelServer anymore.
    duelServer_->updateGameState(
        GameState(*field[0], *field[1],
                  last_accepted_messages_[0], last_accepted_messages_[1]));
}

GameResult Game::GetWinner() const
{
    bool p1_dead = field[0]->isDead();
    bool p2_dead = field[1]->isDead();
    if (!p1_dead && !p2_dead) {
        return GameResult::PLAYING;
    }
    if (p1_dead && p2_dead) {
        return GameResult::DRAW;
    }
    if (p1_dead) {
        return GameResult::P2_WIN;
    }
    if (p2_dead) {
        return GameResult::P1_WIN;
    }

    return GameResult::PLAYING;
}

std::string FormatAckInfo(const vector<int>& ack_info)
{
    stringstream ss;
    int accepted_id = -1;
    bool has_nack = false;
    for (size_t i = 0; i < ack_info.size(); i++) {
        if (ack_info[i] > 0) {
            accepted_id = ack_info[i];
        } else if (ack_info[i] < 0) {
            if (has_nack) {
                ss << ",";
            }
            ss << -ack_info[i];
            has_nack = true;
        }
    }
    stringstream ret;
    if (accepted_id > 0) {
        ret << "ACK=" << accepted_id << " ";
    }
    if (has_nack) {
        ret << "NACK=" << ss.str();
    }
    return ret.str();
}

// TODO(mayah): This should not exist here.
// We must create FrameData, and pass it to connector.
void Game::GetFieldInfo(std::string* player1, std::string* player2) const
{
    std::string f0 = field[0]->GetFieldInfo();
    std::string f1 = field[1]->GetFieldInfo();
    std::string y0 = field[0]->GetYokokuInfo();
    std::string y1 = field[1]->GetYokokuInfo();
    int state0 = field[0]->userState().toDeprecatedState();
    int state1 = field[1]->userState().toDeprecatedState();
    int score0 = field[0]->score();
    int score1 = field[1]->score();
    int ojama0 = field[0]->ojama();
    int ojama1 = field[1]->ojama();
    std::string ack0 = FormatAckInfo(ack_info_[0]);
    std::string ack1 = FormatAckInfo(ack_info_[1]);

    int pos_x_0, pos_y_0, r0;
    int pos_x_1, pos_y_1, r1;
    int dummy1, dummy2;
    PuyoColor c0, c1, dummy3;
    field[0]->GetCurrentPuyo(&pos_x_0, &pos_y_0, &c0, &dummy1, &dummy2, &dummy3, &r0);
    field[1]->GetCurrentPuyo(&pos_x_1, &pos_y_1, &c1, &dummy1, &dummy2, &dummy3, &r1);

    string win0, win1;
    GameResult winner = GetWinner();
    switch (winner) {
    case GameResult::PLAYING:
        break;
    case GameResult::DRAW:
        win0 = "END=1 ";
        win1 = "END=-1 ";
        break;
    case GameResult::P1_WIN:
        win0 = "END=-1 ";
        win1 = "END=1 ";
        break;
    case GameResult::P2_WIN:
        win0 = win1 = "END=0 ";
        break;
    default:
        win0 = win1 = "END=0 ";
        break;
    }

    {
        std::stringstream ss;
        ss << "STATE=" << (state0 + (state1 << 1)) << " "
           << "YF=" << f0 << " "
           << "OF=" << f1 << " "
           << "YP=" << y0 << " "
           << "OP=" << y1 << " "
           << "YX=" << pos_x_0 << " "
           << "YY=" << pos_y_0 << " "
           << "YR=" << r0 << " "
           << "OX=" << pos_x_1 << " "
           << "OY=" << pos_y_1 << " "
           << "OR=" << r1 << " "
           << "YO=" << ojama0 << " "
           << "OO=" << ojama1 << " "
           << "YS=" << score0 << " "
           << "OS=" << score1 << " "
           << win0
           << ack0;
        *player1 = ss.str();
    }

    {
        std::stringstream ss;
        ss << "STATE=" << (state1 + (state0 << 1)) << " "
           << "YF=" << f1 << " "
           << "OF=" << f0 << " "
           << "YP=" << y1 << " "
           << "OP=" << y0 << " "
           << "YX=" << pos_x_1 << " "
           << "YY=" << pos_y_1 << " "
           << "YR=" << r1 << " "
           << "OX=" << pos_x_0 << " "
           << "OY=" << pos_y_0 << " "
           << "OR=" << r0 << " "
           << "YO=" << ojama1 << " "
           << "OO=" << ojama0 << " "
           << "YS=" << score1 << " "
           << "OS=" << score0 << " "
           << win1
           << ack1;
        *player2 = ss.str();
    }

    if (state0 & STATE_YOU_CAN_PLAY) {
        LOG(INFO) << "Player 0 can play.";
    }
    if (state1 & STATE_YOU_CAN_PLAY) {
        LOG(INFO) << "Player 1 can play.";
    }

    // TODO(koichi): Add ACK.
}
