#include "core/puyo_controller.h"

#include <algorithm>
#include <map>
#include <queue>
#include <tuple>
#include <vector>

#include <glog/logging.h>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

namespace {

bool isQuickturn(const PlainField& field, const KumipuyoPos& pos)
{
    DCHECK(pos.r == 0 || pos.r == 2) << pos.r;
    return (field.get(pos.x - 1, pos.y) != PuyoColor::EMPTY && field.get(pos.x + 1, pos.y) != PuyoColor::EMPTY);
}

// Remove redundant key stroke.
void removeRedundantKeySeq(const KumipuyoPos& pos, KeySetSeq* seq)
{
    switch (pos.r) {
    case 0:
        return;
    case 1:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::LEFT_TURN) && (*seq)[1] == KeySet(Key::RIGHT_TURN)) {
            // Remove 2 key strokes.
            seq->removeFront();
            seq->removeFront();
        }
        return;
    case 2:
        return;
    case 3:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::RIGHT_TURN) && (*seq)[1] == KeySet(Key::LEFT_TURN)) {
            seq->removeFront();
            seq->removeFront();
        }
        return;
    default:
        CHECK(false) << "Unknown r: " << pos.r;
        return;
    }
}

KeySetSeq expandButtonDistance(const KeySetSeq& seq)
{
    KeySetSeq result;
    for (size_t i = 0; i < seq.size(); ++i) {
        if (i > 0 && ((seq[i - 1].hasTurnKey() && seq[i].hasTurnKey()) || (seq[i - 1].hasArrowKey() && seq[i].hasArrowKey()))) {
            result.add(KeySet());
        }
        result.add(seq[i]);
    }

    return result;
}

KeySetSeq findKeyStrokeFastpath10(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return KeySetSeq("<,,<,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && (field.height(3) == 11 || field.height(4) == 12))
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath11(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return KeySetSeq("<A,,<,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(3) == 11)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath12(const CoreField& field)
{
    if (field.height(1) <= 9 && field.height(2) <= 9)
        return KeySetSeq("<A,,<,vA,v");
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return KeySetSeq("<A,,<,A,v");
    if (field.height(1) <= 11 && field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v");
    if (field.height(1) <= 11 && field.height(2) == 12 && field.height(3) == 11)
        return KeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath13(const CoreField&)
{
    CHECK(false) << "shouldn't be called";
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath20(const CoreField& field)
{
    if (field.height(2) <= 11)
        return KeySetSeq("<,v");
    if (field.height(2) == 12 && field.height(3) == 11 && field.height(1) <= 12)
        return KeySetSeq("A,,A,,B,<,B,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,B,<,B,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath21(const CoreField& field)
{
    if (field.height(2) <= 11 && field.height(4) <= 11)
        return KeySetSeq("<A,v");
    if (field.height(2) <= 11 && field.height(4) >= 12)
        return KeySetSeq("A,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,B,<,v");
    if (field.height(2) == 12 && field.height(3) == 11 && field.height(4) <= 11)
        return KeySetSeq("A,,A,,B,<,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath22(const CoreField& field)
{
    if (field.height(2) <= 10 && field.height(3) <= 10 && field.height(4) <= 11)
        return KeySetSeq("<A,v,vA,v");
    if (field.height(2) <= 11 && field.height(4) <= 11)
        return KeySetSeq("<A,,A,v");
    if (field.height(2) <= 11 && field.height(4) >= 12)
        return KeySetSeq("A,,A,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath23(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return KeySetSeq("<B,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return KeySetSeq("<A,<,<A,<,<A,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,A,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(3) == 11 && field.height(4) <= 11)
        return KeySetSeq("A,,A,,A,<,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath30(const CoreField&)
{
    return KeySetSeq { KeySet(Key::DOWN) };
}

KeySetSeq findKeyStrokeFastpath31(const CoreField& field)
{
    if (field.height(4) <= 9)
        return KeySetSeq("vA,v");
    if (field.height(4) <= 11)
        return KeySetSeq("A,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("B,,B,,B,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath32(const CoreField& field)
{
    if (field.height(4) <= 6)
        return KeySetSeq("vA,v,vA,v");
    if (field.height(2) <= 6)
        return KeySetSeq("vB,v,vB,v");
    if (field.height(4) <= 11)
        return KeySetSeq("A,,A,v");
    if (field.height(2) <= 11)
        return KeySetSeq("B,,B,v");
    if (field.height(2) >= 12 && field.height(4) >= 12)
        return KeySetSeq("A,,A,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath33(const CoreField& field)
{
    if (field.height(2) <= 9)
        return KeySetSeq("Bv,v");
    if (field.height(2) <= 11)
        return KeySetSeq("B,v");
    if (field.height(2) == 12 && field.height(3) == 11)
        return KeySetSeq("A,,A,,A,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,A,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath40(const CoreField& field)
{
    if (field.height(4) <= 11)
        return KeySetSeq(">,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,A,>,A,v");
    if (field.height(3) == 11 && field.height(4) == 12)
        return KeySetSeq("B,,B,,B,,>,B,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath41(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return KeySetSeq(">A,v");
    if (field.height(4) == 11 && field.height(5) == 12)
        return KeySetSeq(">,B,,B,,B,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(6) <= 12)
        return KeySetSeq("B,,B,,B,>,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath42(const CoreField& field)
{
    if (field.height(4) <= 9 && field.height(5) <= 9)
        return KeySetSeq(">A,v,vA,v");
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return KeySetSeq(">A,,A,v");
    if (field.height(4) <= 11 && field.height(5) >= 12)
        return KeySetSeq(">,B,,B,v");

    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath43(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(2) <= 11)
        return KeySetSeq(">B,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return KeySetSeq("A,,A,,A,>,v");
    if (field.height(3) == 11 && field.height(4) == 12)
        return KeySetSeq("B,,B,,A,,>,v");
    if (field.height(4) <= 11 && field.height(5) >= 12)
        return KeySetSeq(">,B,v");

    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath50(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return KeySetSeq(">,,>,v");
    if (field.height(4) == 11 && field.height(5) == 12)
        return KeySetSeq(">B,>,>B,>,>B,>,,B,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath51(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11)
        return KeySetSeq(">A,,>,v");

    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12)
        return KeySetSeq(">B,>,>,>B,>,>,>,>B,>,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12)
        return KeySetSeq(">B,>,>B,>,>B,>,>,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return KeySetSeq("B,,B,,B,>,,>,v");

    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath52(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 9 && field.height(6) <= 9) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::RIGHT_TURN),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN,Key::RIGHT_TURN),
            KeySet(Key::DOWN)
        };
    }
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::RIGHT_TURN),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT_TURN),
            KeySet(),
            KeySet(Key::DOWN)
        };
    }
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) >= 12) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::LEFT_TURN),
            KeySet(),
            KeySet(Key::DOWN)
        };
    }
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 11)
        return KeySetSeq("B,,B,,B,>,,>,A,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 11 && field.height(6) <= 12)
        return KeySetSeq("B,,B,,A,>,A,>,A,,A,v");

    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath53(const CoreField& field)
{
    if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN),
        };
    }
    if (field.height(4) == 11 && field.height(5) == 12)
        return KeySetSeq(">B,,>B,,A,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 11)
        return KeySetSeq("B,,B,,A,>,,>,,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath60(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11) {
        return KeySetSeq {
            KeySet(Key::RIGHT),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN)
        };
    }
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT, Key::RIGHT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT, Key::RIGHT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN),
        };
    }
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return KeySetSeq("A,,A,,A,>,,>,,>,A,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12)
        return KeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return KeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath61(const CoreField&)
{
    CHECK(false) << "shouldn't be callede";
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath62(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 10 && field.height(6) <= 11)
        return KeySetSeq(">B,,>,,>,B,v");
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) <= 11)
        return KeySetSeq(">B,,>,,>B,,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 11)
        return KeySetSeq("A,,A,,A,>,,>,,>,B,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 11)
        return KeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 11)
        return KeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v");
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath63(const CoreField& field)
{
    if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN)
        };
    }
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12) {
        return KeySetSeq {
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT, Key::LEFT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT, Key::RIGHT_TURN),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT),
            KeySet(Key::RIGHT),
            KeySet(Key::DOWN),
        };
    }
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return KeySetSeq("A,,A,,A,>,,>,,>,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12) {
        // if height(4) == 11, pressing B earlier will cause miscontrolling.
        return KeySetSeq(">B,>,>,>B,>,>A,>,>,>,v");
    }
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return KeySetSeq(">B,>,>B,>,>A,>,>,>,>,>,v");
    return KeySetSeq();
}

} // namespace anomymous

bool PuyoController::isReachable(const CoreField& field, const Decision& decision)
{
    DCHECK(decision.isValid()) << decision.toString();

    static const int checker[6][5] = {
        { 3, 2, 1, 0 },
        { 3, 2, 0 },
        { 3, 0 },
        { 3, 4, 0 },
        { 3, 4, 5, 0 },
        { 3, 4, 5, 6, 0 },
    };

    int checkerIdx = decision.x - 1;
    if (decision.r == 1 && 3 <= decision.x)
        checkerIdx += 1;
    else if (decision.r == 3 && decision.x <= 3)
        checkerIdx -= 1;

    // When decision is valid, this should hold.
    DCHECK(0 <= checkerIdx && checkerIdx < 6) << checkerIdx;

    bool yMightBe13 = field.height(2) >= 12 && field.height(4) >= 12;
    for (int i = 1; checker[checkerIdx][i] != 0; ++i) {
        int x = checker[checkerIdx][i];
        if (field.height(x) <= 11) {
            yMightBe13 = false;
            continue;
        }
        if (field.height(x) == 12) {
            if (yMightBe13)
                continue;
            if (field.height(checker[checkerIdx][i - 1]) == 11) {
                yMightBe13 = true;
                continue;
            }
            if (i - 2 >= 0 && field.height(checker[checkerIdx][i - 2]) == 12) {
                yMightBe13 = true;
                continue;
            }
        }
        return false;
    }

    if (decision.r == 2 && field.height(decision.x) >= 12)
        return false;

    return true;
}

bool PuyoController::isReachableFrom(const PlainField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    return !findKeyStrokeOnlineInternal(field, mks, decision).empty();
}

KeySetSeq PuyoController::findKeyStroke(const CoreField& field, const Decision& decision)
{
    KeySetSeq kss = findKeyStrokeFastpath(field, decision);
    if (!kss.empty())
        return kss;
    return findKeyStrokeOnline(field, KumipuyoMovingState::initialState(), decision);
}

KeySetSeq PuyoController::findKeyStrokeFrom(const CoreField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    if (mks.isInitialPosition())
        return findKeyStroke(field, decision);

    if (!isReachableFrom(field, mks, decision))
        return KeySetSeq();
    return findKeyStrokeByDijkstra(field, mks, decision);
}

typedef KumipuyoMovingState Vertex;

typedef double Weight;
struct Edge {
    Edge() {}
    Edge(const Vertex& src, const Vertex& dest, Weight weight, const KeySet& keySet) :
        src(src), dest(dest), weight(weight), keySet(keySet) {}

    friend bool operator>(const Edge& lhs, const Edge& rhs) {
        if (lhs.weight != rhs.weight) { return lhs.weight > rhs.weight; }
        if (lhs.src != rhs.src) { return lhs.src > rhs.src; }
        return lhs.dest > rhs.dest;
    }

    Vertex src;
    Vertex dest;
    Weight weight;
    KeySet keySet;
};

typedef vector<Edge> Edges;
typedef map<Vertex, tuple<Vertex, KeySet, Weight>> Potential;

KeySetSeq PuyoController::findKeyStrokeByDijkstra(const PlainField& field, const KumipuyoMovingState& initialState, const Decision& decision)
{
    // We don't add KeySet(Key::DOWN) intentionally.
    static const pair<KeySet, double> KEY_CANDIDATES[] = {
        make_pair(KeySet(), 1),
        make_pair(KeySet(Key::LEFT), 1.01),
        make_pair(KeySet(Key::RIGHT), 1.01),
        make_pair(KeySet(Key::LEFT, Key::LEFT_TURN), 1.03),
        make_pair(KeySet(Key::LEFT, Key::RIGHT_TURN), 1.03),
        make_pair(KeySet(Key::RIGHT, Key::LEFT_TURN), 1.03),
        make_pair(KeySet(Key::RIGHT, Key::RIGHT_TURN), 1.03),
        make_pair(KeySet(Key::LEFT_TURN), 1.01),
        make_pair(KeySet(Key::RIGHT_TURN), 1.01),
    };
    static const int KEY_CANDIDATES_SIZE = ARRAY_SIZE(KEY_CANDIDATES);

    static const pair<KeySet, double> KEY_CANDIDATES_WITHOUT_TURN[] = {
        make_pair(KeySet(), 1),
        make_pair(KeySet(Key::LEFT), 1.01),
        make_pair(KeySet(Key::RIGHT), 1.01),
    };
    static const int KEY_CANDIDATES_SIZE_WITHOUT_TURN = ARRAY_SIZE(KEY_CANDIDATES_WITHOUT_TURN);

    static const pair<KeySet, double> KEY_CANDIDATES_WITHOUT_ARROW[] = {
        make_pair(KeySet(), 1),
        make_pair(KeySet(Key::LEFT_TURN), 1.01),
        make_pair(KeySet(Key::RIGHT_TURN), 1.01),
    };
    static const int KEY_CANDIDATES_SIZE_WITHOUT_ARROW = ARRAY_SIZE(KEY_CANDIDATES_WITHOUT_ARROW);

    static const pair<KeySet, double> KEY_CANDIDATES_WITHOUT_TURN_OR_ARROW[] = {
        make_pair(KeySet(), 1),
    };
    static const int KEY_CANDIDATES_SIZE_WITHOUT_TURN_OR_ARROW = ARRAY_SIZE(KEY_CANDIDATES_WITHOUT_TURN_OR_ARROW);

    Potential pot;
    priority_queue<Edge, Edges, greater<Edge> > Q;

    Vertex startV(initialState);
    Q.push(Edge(startV, startV, 0, KeySet()));

    while (!Q.empty()) {
        Edge edge = Q.top(); Q.pop();
        Vertex p = edge.dest;
        Weight curr = edge.weight;

        // already visited?
        if (pot.count(p))
            continue;

        pot[p] = make_tuple(edge.src, edge.keySet, curr);

        // goal.
        if (p.pos.axisX() == decision.x && p.pos.rot() == decision.r) {
            // goal.
            vector<KeySet> kss;
            kss.push_back(KeySet(Key::DOWN));
            while (pot.count(p)) {
                if (p == startV)
                    break;
                const KeySet ks(std::get<1>(pot[p]));
                kss.push_back(ks);
                p = std::get<0>(pot[p]);
            }

            reverse(kss.begin(), kss.end());
            return KeySetSeq(kss);
        }

        if (p.grounded)
            continue;

        const pair<KeySet, double>* candidates;
        int size;
        if (p.restFramesTurnProhibited > 0 && p.restFramesArrowProhibited > 0) {
            candidates = KEY_CANDIDATES_WITHOUT_TURN_OR_ARROW;
            size = KEY_CANDIDATES_SIZE_WITHOUT_TURN_OR_ARROW;
        } else if (p.restFramesTurnProhibited > 0) {
            candidates = KEY_CANDIDATES_WITHOUT_TURN;
            size = KEY_CANDIDATES_SIZE_WITHOUT_TURN;
        } else if (p.restFramesArrowProhibited > 0) {
            candidates = KEY_CANDIDATES_WITHOUT_ARROW;
            size = KEY_CANDIDATES_SIZE_WITHOUT_ARROW;
        } else {
            candidates = KEY_CANDIDATES;
            size = KEY_CANDIDATES_SIZE;
        }

        for (int i = 0; i < size; ++i) {
            const pair<KeySet, double>& candidate = candidates[i];
            KumipuyoMovingState mks(p);
            mks.moveKumipuyo(field, candidate.first);

            if (pot.count(mks))
                continue;
            // TODO(mayah): This is not correct. We'd like to prefer KeySet() to another key sequence a bit.
            Q.push(Edge(p, mks, curr + candidate.second, candidate.first));
        }
    }

    // No way...
    return KeySetSeq();
}

KeySetSeq PuyoController::findKeyStrokeOnline(const PlainField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    KeySetSeq kss = findKeyStrokeOnlineInternal(field, mks, decision);
    removeRedundantKeySeq(mks.pos, &kss);
    return expandButtonDistance(kss);
}

// returns null if not reachable
KeySetSeq PuyoController::findKeyStrokeOnlineInternal(const PlainField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    KeySetSeq ret;
    KumipuyoPos current = mks.pos;

    while (true) {
        if (current.x == decision.x && current.r == decision.r) {
            break;
        }

        // for simplicity, direct child-puyo upwards
        // TODO(yamaguchi): eliminate unnecessary moves
        if (current.r == 1) {
            ret.add(KeySet(Key::LEFT_TURN));
            current.r = 0;
        } else if (current.r == 3) {
            ret.add(KeySet(Key::RIGHT_TURN));
            current.r = 0;
        } else if (current.r == 2) {
            if (isQuickturn(field, current)) {
                // do quick turn
                ret.add(KeySet(Key::RIGHT_TURN));
                ret.add(KeySet(Key::RIGHT_TURN));
                current.y++;
                if (current.y >= 14)
                    return KeySetSeq();
            } else {
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                } else {
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                }
            }
            current.r = 0;
        }

        if (current.x == decision.x) {
            switch (decision.r) {
            case 0:
                break;
            case 1:
                if (field.get(current.x + 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }
                    // turn inversely to avoid kicking wall
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));

                    // TODO(mayah): In some case, this can reject possible key stroke?
                    if (current.y == 13 && field.get(current.x, 12) != PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }

                } else {
                    ret.add(KeySet(Key::RIGHT_TURN));
                }

                break;
            case 3:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));

                    // TODO(mayah): In some case, this can reject possible key stroke?
                    if (current.y == 13 && field.get(current.x, 12) != PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }

                } else {
                    ret.add(KeySet(Key::LEFT_TURN));
                }

                break;
            case 2:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                } else {
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                }

                if (current.y == 13 && field.get(current.x, 12) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }

                break;
            }
            break;
        }

        // direction to move horizontally
        if (decision.x > current.x) {
            // move to right
            if (field.get(current.x + 1, current.y) == PuyoColor::EMPTY) {
                ret.add(KeySet(Key::RIGHT));
                current.x++;
            } else {  // hits a wall
                // climb if possible
                /*
                  aBb
                  .A@
                  .@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13)
                    return KeySetSeq();
                // check "b"
                if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because right side is blocked

                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                    current.y++;
                    if (current.y >= 14)
                        return KeySetSeq();
                    if (field.get(current.x - 1, current.y + 1) == PuyoColor::EMPTY) {
                        ret.add(KeySet(Key::RIGHT_TURN));
                        ret.add(KeySet(Key::RIGHT));
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit right key.
                        ret.add(KeySet(Key::RIGHT_TURN));
                    }
                    ret.add(KeySet(Key::RIGHT_TURN));
                    current.x++;
                } else {
                    return KeySetSeq();
                }
            }
        } else {
            // move to left
            if (field.get(current.x - 1, current.y) == PuyoColor::EMPTY) {
                ret.add(KeySet(Key::LEFT));
                current.x--;
            } else {  // hits a wall
                // climb if possible
                /*
                  bBa
                  @A.
                  @@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) {
                    return KeySetSeq();
                }
                // check "b"
                if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because left side is blocked
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                    current.y++;
                    if (current.y >= 14)
                        return KeySetSeq();
                    if (field.get(current.x + 1, current.y) == PuyoColor::EMPTY) {
                        ret.add(KeySet(Key::LEFT_TURN));
                        ret.add(KeySet(Key::LEFT));
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit left key.
                        ret.add(KeySet(Key::LEFT_TURN));
                    }
                    ret.add(KeySet(Key::LEFT_TURN));
                    current.x--;
                } else {
                    return KeySetSeq();
                }
            }
        }
    }

    ret.add(KeySet(Key::DOWN));
    return ret;
}

KeySetSeq PuyoController::findKeyStrokeFastpath(const CoreField& field, const Decision& decision)
{
    typedef KeySetSeq (*func)(const CoreField&);
    static const func fs[6][4] = {
        { findKeyStrokeFastpath10, findKeyStrokeFastpath11, findKeyStrokeFastpath12, findKeyStrokeFastpath13, },
        { findKeyStrokeFastpath20, findKeyStrokeFastpath21, findKeyStrokeFastpath22, findKeyStrokeFastpath23, },
        { findKeyStrokeFastpath30, findKeyStrokeFastpath31, findKeyStrokeFastpath32, findKeyStrokeFastpath33, },
        { findKeyStrokeFastpath40, findKeyStrokeFastpath41, findKeyStrokeFastpath42, findKeyStrokeFastpath43, },
        { findKeyStrokeFastpath50, findKeyStrokeFastpath51, findKeyStrokeFastpath52, findKeyStrokeFastpath53, },
        { findKeyStrokeFastpath60, findKeyStrokeFastpath61, findKeyStrokeFastpath62, findKeyStrokeFastpath63, },
    };

    CHECK(decision.isValid()) << decision.toString();

    return fs[decision.x - 1][decision.r](field);
}
