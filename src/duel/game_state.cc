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
    if (f0.IsInUserState()) {
        int x1, y1, x2, y2, r;
        PuyoColor c1, c2;
        f0.GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);
        cf0.unsafeSet(x1, y1, puyoColorOf(c1 + '0'));
        cf0.unsafeSet(x2, y2, puyoColorOf(c2 + '0'));
    }
    if (f1.IsInUserState()) {
        int x1, y1, x2, y2, r;
        PuyoColor c1, c2;
        f1.GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);
        cf1.unsafeSet(x1, y1, puyoColorOf(c1 + '0'));
        cf1.unsafeSet(x2, y2, puyoColorOf(c2 + '0'));
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


