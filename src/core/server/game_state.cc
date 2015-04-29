#include "core/server/game_state.h"

#include <sstream>

#include <json/json.h>

#include "core/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo.h"

// TODO(mayah): Do we need CoreField here? PlainField looks OK.

using namespace std;

GameResult GameState::gameResult() const
{
    bool p1Dead = playerGameState_[0].dead;
    bool p2Dead = playerGameState_[1].dead;

    if (p1Dead && p2Dead)
        return GameResult::DRAW;
    if (p1Dead)
        return GameResult::P2_WIN;
    if (p2Dead)
        return GameResult::P1_WIN;
    return GameResult::PLAYING;
}

string GameState::toJson() const
{
    PlainField f[2] = { playerGameState_[0].field, playerGameState_[1].field };

    for (int i = 0; i < 2; ++i) {
        if (playerGameState_[i].playable) {
            const KumipuyoPos& pos = playerGameState_[i].kumipuyoPos;
            const Kumipuyo& kp = playerGameState_[i].kumipuyoSeq.front();
            f[i].setColor(pos.axisX(), pos.axisY(), kp.axis);
            f[i].setColor(pos.childX(), pos.childY(), kp.child);
        }
    }

    Json::Value root;
    root["p1"] = f[0].toString();
    root["s1"] = playerGameState_[0].score;
    root["o1"] = playerGameState_[0].ojama();
    root["n1"] = playerGameState_[0].kumipuyoSeq.toString();
    root["m1"] = playerGameState_[0].message;

    root["p2"] = f[1].toString();
    root["s2"] = playerGameState_[1].score;
    root["o2"] = playerGameState_[1].ojama();
    root["n2"] = playerGameState_[1].kumipuyoSeq.toString();
    root["m2"] = playerGameState_[1].message;

    return root.asString();
}

string GameState::toDebugString() const
{
    PlainField f[2] = { playerGameState_[0].field, playerGameState_[1].field };

    for (int i = 0; i < 2; ++i) {
        if (playerGameState_[i].playable) {
            const KumipuyoPos& pos = playerGameState_[i].kumipuyoPos;
            const Kumipuyo& kp = playerGameState_[i].kumipuyoSeq.front();
            f[i].setColor(pos.axisX(), pos.axisY(), kp.axis);
            f[i].setColor(pos.childX(), pos.childY(), kp.child);
        }
    }

    ostringstream ss;
    ss << FieldPrettyPrinter::toStringFromMultipleFields(
        { f[0], f[1] },
        { playerGameState_[0].kumipuyoSeq, playerGameState_[1].kumipuyoSeq });
    return ss.str();
}

FrameRequest GameState::toFrameRequestFor(int playerId) const
{
    GameResult gr = gameResult();
    if (playerId != 0) {
        gr = toOppositeResult(gr);
    }
    return toFrameRequestFor(playerId, gr);
}

FrameRequest GameState::toFrameRequestFor(int playerId, GameResult forceSetGameResult) const
{
    FrameRequest req;
    req.frameId = frameId_;
    req.gameResult = forceSetGameResult;
    for (int i = 0; i < 2; ++i) {
        PlayerFrameRequest& pfr = req.playerFrameRequest[i];
        const PlayerGameState& pgs = playerGameState_[playerId == 0 ? i : 1 - i];
        pfr.field = pgs.field;
        pfr.kumipuyoSeq = pgs.kumipuyoSeq;
        pfr.kumipuyoPos = pgs.kumipuyoPos;
        pfr.event = pgs.event;
        pfr.score = pgs.score;
        pfr.ojama = pgs.ojama();
    }

    return req;
}
