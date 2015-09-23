#include "core/server/game_state.h"

#include <cstdio>
#include <sstream>

#include "core/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo.h"

// TODO(mayah): Do we need CoreField here? PlainField looks OK.

using namespace std;

// GameState::GameState(const std::string& json)
GameState::GameState(const Json::Value& root)
{
  frameId_ = root.get("f", -1).asInt();

  playerGameState_[0].field = PlainField(root.get("p1", "").asString());
  playerGameState_[0].score = root.get("s1", 0).asInt();
  playerGameState_[0].pendingOjama = root.get("po1", 0).asInt();
  playerGameState_[0].fixedOjama = root.get("fo1", 0).asInt();
  playerGameState_[0].kumipuyoSeq = KumipuyoSeq(root.get("n1", "").asString());
  playerGameState_[0].kumipuyoPos = KumipuyoPos(root.get("pos1", "").asString());
  playerGameState_[0].message = root.get("m1", "").asString();
  playerGameState_[0].dead = root.get("d1", false).asBool();
  playerGameState_[0].playable = root.get("playable1", false).asBool();
  playerGameState_[0].event = UserEvent(root.get("e1", "").asString());

  playerGameState_[1].field = PlainField(root.get("p2", "").asString());
  playerGameState_[1].score = root.get("s2", 0).asInt();
  playerGameState_[1].pendingOjama = root.get("po2", 0).asInt();
  playerGameState_[1].fixedOjama = root.get("fo2", 0).asInt();
  playerGameState_[1].kumipuyoSeq = KumipuyoSeq(root.get("n2", "").asString());
  playerGameState_[1].kumipuyoPos = KumipuyoPos(root.get("pos2", "").asString());
  playerGameState_[1].message = root.get("m2", "").asString();
  playerGameState_[1].dead = root.get("d2", false).asBool();
  playerGameState_[1].playable = root.get("playable2", false).asBool();
  playerGameState_[1].event = UserEvent(root.get("e2", "").asString());
}


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
    root["f"] = frameId_;

    root["p1"] = f[0].toString();
    root["s1"] = playerGameState_[0].score;
    root["o1"] = playerGameState_[0].ojama();
    root["n1"] = playerGameState_[0].kumipuyoSeq.toString();
    root["m1"] = playerGameState_[0].message;
    root["pos1"] = playerGameState_[0].kumipuyoPos.toString();
    root["playable1"] = playerGameState_[0].playable;
    root["d1"] = playerGameState_[0].dead;
    root["e1"] = playerGameState_[0].event.toString();
    root["po1"] = playerGameState_[0].pendingOjama;
    root["fo1"] = playerGameState_[0].fixedOjama;

    root["p2"] = f[1].toString();
    root["s2"] = playerGameState_[1].score;
    root["o2"] = playerGameState_[1].ojama();
    root["n2"] = playerGameState_[1].kumipuyoSeq.toString();
    root["m2"] = playerGameState_[1].message;
    root["pos2"] = playerGameState_[1].kumipuyoPos.toString();
    root["playable2"] = playerGameState_[1].playable;
    root["d2"] = playerGameState_[1].dead;
    root["e2"] = playerGameState_[1].event.toString();
    root["po2"] = playerGameState_[1].pendingOjama;
    root["fo2"] = playerGameState_[1].fixedOjama;

    Json::FastWriter writer;
    return writer.write(root);
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
