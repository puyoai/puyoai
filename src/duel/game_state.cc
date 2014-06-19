#include "duel/game_state.h"

#include <sstream>
#include "core/field/core_field.h"
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
        cf0.unsafeSet(pos.axisX(), pos.axisY(), puyoColorOf(kp.axis + '0'));
        cf0.unsafeSet(pos.childX(), pos.childY(), puyoColorOf(kp.child + '0'));
    }
    if (f1.userPlayable()) {
        KumipuyoPos pos = f1.kumipuyoPos();
        Kumipuyo kp = f1.kumipuyo();
        cf1.unsafeSet(pos.axisX(), pos.axisY(), puyoColorOf(kp.axis + '0'));
        cf1.unsafeSet(pos.childX(), pos.childY(), puyoColorOf(kp.child + '0'));
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
