#include "game.h"

#include <cstdlib>
#include <iostream>
#include <glog/logging.h>
#include <sstream>
#include <string>
#include <vector>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/server/connector/data.h"
#include "core/server/connector/game_log.h"
#include "core/state.h"
#include "duel/duel_server.h"
#include "duel/field_realtime.h"
#include "duel/frame_context.h"
#include "duel/game_state.h"
#include "duel/sequence_generator.h"
#include "duel/user_input.h"

using namespace std;

Game::Game(DuelServer* duelServer, UserInput* userInput) :
    duelServer_(duelServer),
    userInput_(userInput)
{
    KumipuyoSeq seq = generateSequence();
    LOG(INFO) << "Puyo sequence=" << seq.toString();

    for (int i = 0; i < 2; i++) {
        field[i].reset(new FieldRealtime(i, seq));
        latest_decision_[i] = Decision::NoInputDecision();
    }
}

Game::~Game()
{
}

/**
 * Updates decision when an applicable one is found.
 * Returns:
 *   if there is an accepted decision:
 *     its index in the given data array.
 *   else:
 *     -1
 */
int UpdateDecision(const PlayerLog& data, FieldRealtime* field, Decision* decision)
{
    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = data.received_data.size(); i > 0; ) {
        i--;

        // We don't need ACK/NACK for ID only messages.
        if (data.received_data[i].decision.x == 0 &&
            data.received_data[i].decision.r == 0) {
            continue;
        }

        Decision d = Decision(data.received_data[i].decision.x, data.received_data[i].decision.r);

        if (d.isValid()) {
            Key key = field->GetKey(d);
            if (key != KEY_NONE) {
                *decision = d;
                return i;
            }
        } else if (d.x == -1 && d.r == -1) {
            *decision = d;
            return i;
        }
    }
    return -1;
}

void Game::Play(const vector<PlayerLog>& data, GameLog* log)
{
    for (int i = 0; i < data.size(); i++) {
        FieldRealtime* me = field[i].get();
        string accepted_message;
        Key key = KEY_NONE;

        if (data[i].is_human) {
            if (userInput_) {
                key = userInput_->getKey();
            }
        } else {
            int accepted_index = UpdateDecision(data[i], field[i].get(), &latest_decision_[i]);

            // Take care of ack_info.
            ack_info_[i] = vector<int>(data[i].received_data.size(), 0);
            for (int j = 0; j < data[i].received_data.size(); j++) {
                const ReceivedData& d = data[i].received_data[j];
                // This case does not require ack.
                if (d.decision.x == 0 && d.decision.r == 0) {
                    continue;
                }
                if (j == accepted_index) {
                    ack_info_[i][j] = d.frame_id;
                } else {
                    ack_info_[i][j] = -d.frame_id;
                }
            }

            if (accepted_index != -1) {
                accepted_message = data[i].received_data[accepted_index].msg;
            }

            key = me->GetKey(latest_decision_[i]);
        }

        PlayerLog player_log = data[i];
        FrameContext context;
        me->PlayOneFrame(key, &player_log, &context);

        FieldRealtime* opponent = field[0].get() == me ? field[1].get() : field[0].get();
        context.apply(me, opponent);

        {
            int moving_x;
            int moving_y;
            int moving_r;
            PuyoColor c1, c2;
            {
                int x2, y2;
                me->GetCurrentPuyo(&moving_x, &moving_y, &c1,
                                   &x2, &y2, &c2, &moving_r);
            }
            player_log.execution_data.keys.push_back(key);
            player_log.execution_data.kumipuyoPos.x = moving_x;
            player_log.execution_data.kumipuyoPos.y = moving_y;
            player_log.execution_data.kumipuyoPos.r = moving_r;
            player_log.execution_data.kumipuyo.axis = c1;
            player_log.execution_data.kumipuyo.child = c2;
            player_log.execution_data.landed = me->userState().grounded;
            log->log.push_back(player_log);
        }

        // Clear current key input if the move is done.
        if (me->userState().grounded) {
            latest_decision_[i] = Decision::NoInputDecision();
        }

        if (accepted_message != "") {
            last_accepted_messages_[me->playerId()] = accepted_message;
        }
    }

    duelServer_->updateGameState(GameState(*field[0], *field[1],
                                           last_accepted_messages_[0], last_accepted_messages_[1]));
}

GameResult Game::GetWinner(int* scores) const
{
    scores[0] = field[0]->score();
    scores[1] = field[1]->score();
    bool p1_dead = field[0]->isDead();
    bool p2_dead = field[1]->isDead();
    if (!p1_dead && !p2_dead) {
        return PLAYING;
    }
    if (p1_dead && p2_dead) {
        return DRAW;
    }
    if (p1_dead) {
        return P2_WIN;
    }
    if (p2_dead) {
        return P1_WIN;
    }
    return PLAYING;
}

std::string FormatAckInfo(const vector<int>& ack_info)
{
    stringstream ss;
    int accepted_id = -1;
    bool has_nack = false;
    for (int i = 0; i < ack_info.size(); i++) {
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
    int scores[2];
    int winner = GetWinner(scores);
    switch (winner) {
    case -1:
        break;
    case 0:
        win0 = "END=1 ";
        win1 = "END=-1 ";
        break;
    case 1:
        win0 = "END=-1 ";
        win1 = "END=1 ";
        break;
    case 2:
        win0 = win1 = "END=0 ";
        break;
    default:
        abort();
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
