#include "core/puyo_controller.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <glog/logging.h>

#include "base/base.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/key.h"
#include "core/kumipuyo_pos.h"
#include "core/kumipuyo_moving_state.h"
#include "core/puyo_color.h"

using namespace std;

namespace {

bool isQuickturn(const CoreField& field, const KumipuyoPos& pos)
{
    DCHECK(pos.r == 0 || pos.r == 2) << pos.r;
    return (field.color(pos.x - 1, pos.y) != PuyoColor::EMPTY && field.color(pos.x + 1, pos.y) != PuyoColor::EMPTY);
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

PrecedeKeySetSeq findKeyStrokeFastpath10(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return PrecedeKeySetSeq("<,,<,v", '<', "<,<,<,<,<,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v", '<', "<A,<,<A,<,<B,<,<,<,<B,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && (field.height(3) == 11 || field.height(4) == 12))
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v", '<', "<A,<,<A,<,<B,<,<,<,<B,<,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath11(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return PrecedeKeySetSeq("<A,,<,v", '<', "<A,<,<,<,<,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,v", '<', "<A,<,<A,<,<B,<,<,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,v", '<', "<A,<,<A,<,<B,<,<,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(3) == 11)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,v", '<', "<A,<,<A,<,<B,<,<,<,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath12(const CoreField& field)
{
    if (field.height(1) <= 9 && field.height(2) <= 9)
        return PrecedeKeySetSeq("<A,,<,vA,v", '<', "<A,<,<,<A,<,v");
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return PrecedeKeySetSeq("<A,,<,A,v", '<', "<A,<,<,<A,<,v");
    if (field.height(1) <= 11 && field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v", '<', "<A,<,<A,<,<B,<,<,<,<A,<,v");
    if (field.height(1) <= 11 && field.height(2) == 12 && field.height(3) == 11)
        return PrecedeKeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v", '<', "<A,<,<A,<,<B,<,<,<,<A,<,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath13(const CoreField&)
{
    CHECK(false) << "shouldn't be called";
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath20(const CoreField& field)
{
    if (field.height(2) <= 11)
        return PrecedeKeySetSeq("<,v");
    if (field.height(2) == 12 && field.height(3) == 11 && field.height(1) <= 12)
        return PrecedeKeySetSeq("A,,A,,B,<,B,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,B,<,B,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath21(const CoreField& field)
{
    if (field.height(2) <= 11 && field.height(4) <= 11)
        return PrecedeKeySetSeq("<A,v");
    if (field.height(2) <= 11 && field.height(4) >= 12)
        return PrecedeKeySetSeq("A,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,B,<,v");
    if (field.height(2) == 12 && field.height(3) == 11 && field.height(4) <= 11)
        return PrecedeKeySetSeq("A,,A,,B,<,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath22(const CoreField& field)
{
    if (field.height(2) <= 10 && field.height(3) <= 10 && field.height(4) <= 11)
        return PrecedeKeySetSeq("<A,v,vA,v");
    if (field.height(2) <= 11 && field.height(4) <= 11)
        return PrecedeKeySetSeq("<A,,A,v");
    if (field.height(2) <= 11 && field.height(4) >= 12)
        return PrecedeKeySetSeq("A,,A,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath23(const CoreField& field)
{
    if (field.height(1) <= 11 && field.height(2) <= 11)
        return PrecedeKeySetSeq("<B,v");
    if (field.height(1) == 12 && field.height(2) == 11)
        return PrecedeKeySetSeq("<A,<,<A,<,<A,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,A,<,v");
    if (field.height(1) <= 12 && field.height(2) == 12 && field.height(3) == 11 && field.height(4) <= 11)
        return PrecedeKeySetSeq("A,,A,,A,<,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath30(const CoreField&)
{
    return PrecedeKeySetSeq("v", 'v', "v");
}

PrecedeKeySetSeq findKeyStrokeFastpath31(const CoreField& field)
{
    if (field.height(4) <= 9)
        return PrecedeKeySetSeq("vA,v", 'v', "vA,v");
    if (field.height(4) <= 11)
        return PrecedeKeySetSeq("A,v");
    if (field.height(2) >= 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("B,,B,,B,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath32(const CoreField& field)
{
    if (field.height(4) <= 6)
        return PrecedeKeySetSeq("vA,v,vA,v", 'v', "vA,v,vA,v");
    if (field.height(2) <= 6)
        return PrecedeKeySetSeq("vB,v,vB,v", 'v', "vB,v,vB,v");
    if (field.height(4) <= 11)
        return PrecedeKeySetSeq("A,,A,v");
    if (field.height(2) <= 11)
        return PrecedeKeySetSeq("B,,B,v");
    if (field.height(2) >= 12 && field.height(4) >= 12)
        return PrecedeKeySetSeq("A,,A,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath33(const CoreField& field)
{
    if (field.height(2) <= 9)
        return PrecedeKeySetSeq("Bv,v", 'v', "Bv,v");
    if (field.height(2) <= 11)
        return PrecedeKeySetSeq("B,v");
    if (field.height(2) == 12 && field.height(3) == 11)
        return PrecedeKeySetSeq("A,,A,,A,v");
    if (field.height(2) == 12 && field.height(4) >= 12)
        return PrecedeKeySetSeq("A,,A,,A,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath40(const CoreField& field)
{
    if (field.height(4) <= 11)
        return PrecedeKeySetSeq(">,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,A,>,A,v");
    if (field.height(2) >= 13 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,A,,A,,v");
    if (field.height(3) == 11 && field.height(4) == 12)
        return PrecedeKeySetSeq("B,,B,,B,,>,B,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath41(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return PrecedeKeySetSeq(">A,v");
    if (field.height(4) == 11 && field.height(5) == 12)
        return PrecedeKeySetSeq(">,B,,B,,B,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq("B,,B,,B,>,v");
    if (field.height(2) >= 12 && field.height(4) == 12 && field.height(5) <= 12)
        return PrecedeKeySetSeq("A,,A,,B,,>,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath42(const CoreField& field)
{
    if (field.height(4) <= 9 && field.height(5) <= 9)
        return PrecedeKeySetSeq(">A,v,vA,v");
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return PrecedeKeySetSeq(">A,,A,v");
    if (field.height(4) <= 11 && field.height(5) >= 12)
        return PrecedeKeySetSeq(">,B,,B,v");

    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath43(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(2) <= 11)
        return PrecedeKeySetSeq(">B,v");
    if (field.height(2) == 12 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,A,>,v");
    if (field.height(2) == 13 && field.height(4) == 12)
        return PrecedeKeySetSeq("A,,A,,A,,v");
    if (field.height(3) == 11 && field.height(4) == 12)
        return PrecedeKeySetSeq("B,,B,,A,,>,v");
    if (field.height(4) <= 11 && field.height(5) >= 12)
        return PrecedeKeySetSeq(">,B,v");

    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath50(const CoreField& field)
{
    if (field.height(4) <= 9 && field.height(5) <= 9 && field.height(6) <= 9)
        return PrecedeKeySetSeq(">,,>,v", '>', ">A,>,>,>B,>,v");
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) >= 12)
        return PrecedeKeySetSeq(">,,>,v", '>', ">,>,>,>,>,v");
    if (field.height(4) <= 11 && field.height(5) <= 11)
        return PrecedeKeySetSeq(">,,>,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,>,>B,>,>B,>,,B,v", '>', ">B,>,>B,>,>B,>,,B,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) >= 12)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,,A,v", '>', ">B,>,>B,>,>A,>,,A,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,>,A,>,,v");
    if (field.height(2) >= 12 && field.height(4) == 12 && field.height(5) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,,>,,A,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath51(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">A,,>,v", '>', ">A,>,>,>,>,v");
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12)
        return PrecedeKeySetSeq(">B,>,>,>B,>,>,>,>B,>,v", '>', ">B,>,>,>B,>,>,>,>B,>,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq(">B,>,>B,>,>B,>,>,>,v", '>', ">B,>,>B,>,>B,>,>,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq("B,,B,,B,>,,>,v");

    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath52(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 9 && field.height(6) <= 9)
        return PrecedeKeySetSeq(">A,,>,vA,v", '>', ">A,>,>,>,vA,v");
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">A,,>,A,,v");
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) >= 12)
        return PrecedeKeySetSeq(">B,,>,B,,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 11)
        return PrecedeKeySetSeq("B,,B,,B,>,,>,A,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 11 && field.height(6) <= 12)
        return PrecedeKeySetSeq("B,,B,,A,>,A,>,A,,A,v");
    if (field.height(2) >= 13 && field.height(4) == 12 && field.height(5) <= 11 && field.height(6) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,,>,B,v");

    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath53(const CoreField& field)
{
    if (field.height(4) <= 6 && field.height(5) <= 6 && field.height(6) <= 6)
        return PrecedeKeySetSeq(">B,,>,v", '>', ">A,>,>,>,>,vA,v,vA,v");
    if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11)
        return PrecedeKeySetSeq(">B,,>,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,>,,>,,v");
    if (field.height(2) >= 13 && field.height(4) == 12 && field.height(5) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,,>,v");
    if (field.height(4) == 11 && field.height(5) == 12)
        return PrecedeKeySetSeq(">B,,>B,,A,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 11)
        return PrecedeKeySetSeq("B,,B,,A,>,,>,,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath60(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">,,>,,>,v", '>', ">,>,>,>,>,>,>,v");
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12)
        return PrecedeKeySetSeq(">B,>,>,>B,>,>A,>,>A,>,v", '>', ">B,>,>,>B,>,>A,>,>A,>,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,>,,>,,>,A,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v", '>', ">B,>,>B,>,>A,>,>,>,>A,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v", '>', ">B,>,>B,>,>A,>,>,>,>A,>,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath61(const CoreField&)
{
    CHECK(false) << "shouldn't be called";
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath62(const CoreField& field)
{
    if (field.height(4) <= 11 && field.height(5) <= 10 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,,>,,>,B,v", '>', ">B,>,>,>,>,>B,v");
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,,>,,>B,,v", '>', ">B,>,>,>,>,>B,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 11)
        return PrecedeKeySetSeq("A,,A,,A,>,,>,,>,B,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v", '>', ">B,>,>B,>,>A,>,>,>,>B,>,v");
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v", '>', ">B,>,>B,>,>A,>,>,>,>B,>,v");
    return PrecedeKeySetSeq();
}

PrecedeKeySetSeq findKeyStrokeFastpath63(const CoreField& field)
{
    if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11)
        return PrecedeKeySetSeq(">B,,>,,>,v", '>', ">B,>,>,>,>,v");
    if (field.height(4) <= 11 && field.height(5) == 11 && field.height(6) == 12)
        return PrecedeKeySetSeq(">B,>,>,>B,>,>A,>,>,>,v", '>', ">B,>,>,>B,>,>A,>,>,>,v");
    if (field.height(2) == 12 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq("A,,A,,A,>,,>,,>,v");
    if (field.height(4) == 11 && field.height(5) == 12 && field.height(6) <= 12) {
        // if height(4) == 11, pressing B earlier will cause miscontrolling.
        return PrecedeKeySetSeq(">B,>,>,>B,>,>A,>,>,>,v", '>', ">B,>,>,>B,>,>A,>,>,>,v");
    }
    if (field.height(3) == 11 && field.height(4) == 12 && field.height(5) <= 12 && field.height(6) <= 12)
        return PrecedeKeySetSeq(">B,>,>B,>,>A,>,>,>,>,>,v", '>', ">B,>,>B,>,>A,>,>,>,>,>,v");
    return PrecedeKeySetSeq();
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

bool PuyoController::isReachableFrom(const CoreField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    return !findKeyStrokeOnlineInternal(field, mks, decision).empty();
}

PrecedeKeySetSeq PuyoController::findKeyStroke(const CoreField& field, const Decision& decision)
{
    PrecedeKeySetSeq pkss = findKeyStrokeFastpath(field, decision);
    if (!pkss.empty())
        return pkss;

    return PrecedeKeySetSeq(findKeyStrokeOnline(field, KumipuyoMovingState::initialState(), decision));
}

KeySetSeq PuyoController::findKeyStrokeFrom(const CoreField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    if (mks.isInitialPosition())
        return findKeyStroke(field, decision).seq();

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

KeySetSeq PuyoController::findKeyStrokeByDijkstra(const CoreField& field, const KumipuyoMovingState& initialState, const Decision& decision)
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

    PlainField plainField = field.toPlainField();

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
            mks.moveKumipuyo(plainField, candidate.first);

            if (pot.count(mks))
                continue;
            // TODO(mayah): This is not correct. We'd like to prefer KeySet() to another key sequence a bit.
            Q.push(Edge(p, mks, curr + candidate.second, candidate.first));
        }
    }

    // No way...
    return KeySetSeq();
}

KeySetSeq PuyoController::findKeyStrokeOnline(const CoreField& field, const KumipuyoMovingState& mks, const Decision& decision)
{
    KeySetSeq kss = findKeyStrokeOnlineInternal(field, mks, decision);
    removeRedundantKeySeq(mks.pos, &kss);
    return expandButtonDistance(kss);
}

// returns null if not reachable
KeySetSeq PuyoController::findKeyStrokeOnlineInternal(const CoreField& field, const KumipuyoMovingState& mks, const Decision& decision)
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
                if (field.color(current.x - 1, current.y) != PuyoColor::EMPTY) {
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
                if (field.color(current.x + 1, current.y) != PuyoColor::EMPTY) {
                    if (field.color(current.x + 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.color(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }
                    // turn inversely to avoid kicking wall
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));

                    // TODO(mayah): In some case, this can reject possible key stroke?
                    if (current.y == 13 && field.color(current.x, 12) != PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }

                } else {
                    ret.add(KeySet(Key::RIGHT_TURN));
                }

                break;
            case 3:
                if (field.color(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    if (field.color(current.x - 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.color(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));

                    // TODO(mayah): In some case, this can reject possible key stroke?
                    if (current.y == 13 && field.color(current.x, 12) != PuyoColor::EMPTY) {
                        return KeySetSeq();
                    }

                } else {
                    ret.add(KeySet(Key::LEFT_TURN));
                }

                break;
            case 2:
                if (field.color(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                } else {
                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                }

                if (current.y == 13 && field.color(current.x, 12) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }

                break;
            }
            break;
        }

        // direction to move horizontally
        if (decision.x > current.x) {
            // move to right
            if (field.color(current.x + 1, current.y) == PuyoColor::EMPTY) {
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
                if (field.color(current.x + 1, current.y + 1) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }
                if (field.color(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because right side is blocked

                    ret.add(KeySet(Key::LEFT_TURN));
                    ret.add(KeySet(Key::LEFT_TURN));
                    current.y++;
                    if (current.y >= 14)
                        return KeySetSeq();
                    if (field.color(current.x - 1, current.y + 1) == PuyoColor::EMPTY) {
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
            if (field.color(current.x - 1, current.y) == PuyoColor::EMPTY) {
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
                if (field.color(current.x - 1, current.y + 1) != PuyoColor::EMPTY) {
                    return KeySetSeq();
                }
                if (field.color(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because left side is blocked
                    ret.add(KeySet(Key::RIGHT_TURN));
                    ret.add(KeySet(Key::RIGHT_TURN));
                    current.y++;
                    if (current.y >= 14)
                        return KeySetSeq();
                    if (field.color(current.x + 1, current.y) == PuyoColor::EMPTY) {
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

PrecedeKeySetSeq PuyoController::findKeyStrokeFastpath(const CoreField& field, const Decision& decision)
{
    typedef PrecedeKeySetSeq (*func)(const CoreField&);
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
