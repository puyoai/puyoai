#include "duel/game_state.h"

#include <sstream>
#include "core/field/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo.h"
#include "duel/field_realtime.h"

// TODO(mayah): Do we need CoreField here? PlainField looks OK.

using namespace std;

static string escapeMessage(const string& s)
{
    ostringstream ss;
    for (char c: s) {
        if (c == '\"')
            ss << "\\\"";
        else
            ss << c;
    }
    return ss.str();
}

GameState::GameState(const KumipuyoSeq& seq) :
    field_ { FieldRealtime(0, seq), FieldRealtime(1, seq) },
    ackFrameId_ { -1, -1 }
{
}

GameResult GameState::gameResult() const
{
    bool p1_dead = field(0).isDead();
    bool p2_dead = field(1).isDead();

    if (!p1_dead && !p2_dead)
        return GameResult::PLAYING;
    if (p1_dead && p2_dead)
        return GameResult::DRAW;
    if (p1_dead)
        return GameResult::P2_WIN;
    if (p2_dead)
        return GameResult::P1_WIN;
    return GameResult::PLAYING;
}

string GameState::toJson() const
{
    const FieldRealtime& f0 = field(0);
    const FieldRealtime& f1 = field(1);

    // Set the current field data to GameState.
    CoreField cf0(f0.field());
    CoreField cf1(f1.field());
    if (f0.userPlayable()) {
        KumipuyoPos pos = f0.kumipuyoPos();
        Kumipuyo kp = f0.kumipuyo();
        cf0.unsafeSet(pos.axisX(), pos.axisY(), kp.axis);
        cf0.unsafeSet(pos.childX(), pos.childY(), kp.child);
    }
    if (f1.userPlayable()) {
        KumipuyoPos pos = f1.kumipuyoPos();
        Kumipuyo kp = f1.kumipuyo();
        cf1.unsafeSet(pos.axisX(), pos.axisY(), kp.axis);
        cf1.unsafeSet(pos.childX(), pos.childY(), kp.child);
    }

    ostringstream ss;

    ss << "{";
    ss << "\"p1\": \"" << cf0.toString() << "\",\n";
    ss << "\"s1\": " << f0.score() << ",\n";
    ss << "\"o1\": " << f0.ojama() << ",\n";
    ss << "\"n1\": \"" << f0.kumipuyoSeq().toString() << "\",\n";
    ss << "\"m1\": \"" << escapeMessage(message(0)) << "\",\n";

    ss << "\"p2\": \"" << cf1.toString() << "\",\n";
    ss << "\"s2\": " << f1.score() << ",\n";
    ss << "\"o2\": " << f1.ojama() << ",\n";
    ss << "\"n2\": \"" << f1.kumipuyoSeq().toString() << "\",\n";
    ss << "\"m2\": \"" << escapeMessage(message(1)) << "\",\n";
    ss << "}";

    return ss.str();
}

string GameState::toDebugString() const
{
    const FieldRealtime& f0 = field(0);
    const FieldRealtime& f1 = field(1);

    // Set the current field data to GameState.
    CoreField cf0(f0.field());
    CoreField cf1(f1.field());
    if (f0.userPlayable()) {
        KumipuyoPos pos = f0.kumipuyoPos();
        Kumipuyo kp = f0.kumipuyo();
        cf0.unsafeSet(pos.axisX(), pos.axisY(), kp.axis);
        cf0.unsafeSet(pos.childX(), pos.childY(), kp.child);
    }
    if (f1.userPlayable()) {
        KumipuyoPos pos = f1.kumipuyoPos();
        Kumipuyo kp = f1.kumipuyo();
        cf1.unsafeSet(pos.axisX(), pos.axisY(), kp.axis);
        cf1.unsafeSet(pos.childX(), pos.childY(), kp.child);
    }

    ostringstream ss;
    ss << FieldPrettyPrinter::toStringFromMultipleFields(f0.field(), f0.kumipuyoSeq(),
                                                         f1.field(), f1.kumipuyoSeq());
    return ss.str();
}

FrameRequest GameState::toFrameRequest(int playerId, int frameId) const
{
    return toFrameRequest(playerId, frameId, gameResult());
}

FrameRequest GameState::toFrameRequest(int playerId, int frameId, GameResult forceSetGameResult) const
{
    FrameRequest req;
    req.frameId = frameId;
    req.gameResult = playerId == 0 ? forceSetGameResult : toOppositeResult(forceSetGameResult);
    for (int i = 0; i < 2; ++i) {
        PlayerFrameRequest& pfr = req.playerFrameRequest[i];
        const FieldRealtime& f = field(playerId == 0 ? i : 1 - i);
        pfr.field = f.field();
        pfr.kumipuyoSeq = KumipuyoSeq { f.kumipuyo(0), f.kumipuyo(1), f.kumipuyo(2) };
        pfr.kumipuyoPos = f.kumipuyoPos();
        pfr.state = f.userState();
        pfr.score = f.score();
        pfr.ojama = f.ojama();
        pfr.ackFrameId = ackFrameId(playerId == 0 ? i : 1 - i);
        pfr.nackFrameIds = nackFrameIds(playerId == 0 ? i : 1 - i);
    }

    return req;
}
