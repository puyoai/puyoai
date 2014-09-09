#include "core/puyo_controller.h"

#include <algorithm>
#include <map>
#include <queue>
#include <tuple>
#include <vector>

#include <glog/logging.h>

#include "core/decision.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

namespace {

bool isQuickturn(const CoreField& field, const KumipuyoPos& pos)
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

void expandButtonDistance(KeySetSeq* seq)
{
    KeySetSeq kss;
    bool shouldSkipButton = false;
    for (const auto& ks : *seq) {
        bool hasTurnKey = ks.hasTurnKey();
        if (shouldSkipButton && hasTurnKey)
            kss.add(KeySet());
        kss.add(ks);
        if (hasTurnKey)
            shouldSkipButton = hasTurnKey;
    }

    *seq = kss;
}

}

void PuyoController::moveKumipuyo(const CoreField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    // TODO(mayah): Which key is consumed first? turn? arrow?
    moveKumipuyoByArrowKey(field, keySet, mks, downAccepted);

    if (mks->restFramesToAcceptQuickTurn > 0)
        mks->restFramesToAcceptQuickTurn--;

    bool needsFreefallProcess = true;
    if (mks->restFramesTurnProhibited > 0) {
        mks->restFramesTurnProhibited--;
    } else {
        moveKumipuyoByTurnKey(field, keySet, mks, &needsFreefallProcess);
    }
    if (!keySet.hasKey(Key::DOWN) && needsFreefallProcess)
        moveKumipuyoByFreefall(field, mks);
}

void PuyoController::moveKumipuyoByArrowKey(const CoreField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    DCHECK(mks);
    DCHECK(!mks->grounded) << "Grounded puyo cannot be moved.";

    // Only one key will be accepted.
    // When DOWN + RIGHT or DOWN + LEFT are simultaneously input, DOWN should be ignored.

    if (keySet.hasKey(Key::RIGHT)) {
        if (field.get(mks->pos.axisX() + 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() + 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x++;
        }
        return;
    }

    if (keySet.hasKey(Key::LEFT)) {
        if (field.get(mks->pos.axisX() - 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() - 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x--;
        }
        return;
    }

    if (keySet.hasKey(Key::DOWN)) {
        if (downAccepted)
            *downAccepted = true;
        if (mks->restFramesForFreefall > 0) {
            mks->restFramesForFreefall = 0;
            return;
        }

        mks->restFramesForFreefall = 0;
        if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
            mks->pos.y--;
            return;
        }

        // Grounded.
        mks->grounded = true;
        return;
    }
}

void PuyoController::moveKumipuyoByTurnKey(const CoreField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* needsFreefallProcess)
{
    DCHECK_EQ(0, mks->restFramesTurnProhibited) << mks->restFramesTurnProhibited;

    if (keySet.hasKey(Key::RIGHT_TURN)) {
        mks->restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                mks->restFramesForFreefall = FRAMES_FREE_FALL;
                *needsFreefallProcess = false;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                return;
            }

            if (mks->pos.y < 13) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.y++;
                mks->restFramesForFreefall = FRAMES_FREE_FALL;
                *needsFreefallProcess = false;
                return;
            }

            // The axis cannot be moved to 14th line.
            return;
        case 2:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            mks->pos.r = (mks->pos.r + 1) % 4;
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }

    if (keySet.hasKey(Key::LEFT_TURN)) {
        mks->restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                mks->restFramesForFreefall = FRAMES_FREE_FALL;
                *needsFreefallProcess = false;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            mks->pos.r = (mks->pos.r + 3) % 4;
            return;
        case 2:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                return;
            }

            if (mks->pos.y < 13) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.y++;
                mks->restFramesForFreefall = FRAMES_FREE_FALL;
                *needsFreefallProcess = false;
                return;
            }

            // The axis cannot be moved to 14th line.
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }
}

void PuyoController::moveKumipuyoByFreefall(const CoreField& field, MovingKumipuyoState* mks)
{
    DCHECK(!mks->grounded);

    if (mks->restFramesForFreefall > 1) {
        mks->restFramesForFreefall--;
        return;
    }

    mks->restFramesForFreefall = FRAMES_FREE_FALL;
    if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
        field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
        mks->pos.y--;
        return;
    }

    mks->grounded = true;
    return;
}

bool PuyoController::isReachable(const CoreField& field, const Decision& decision)
{
    if (isReachableFastpath(field, decision))
        return true;

    // slowpath
    return isReachableFrom(field, MovingKumipuyoState(KumipuyoPos::initialPos()), decision);
}

bool PuyoController::isReachableFastpath(const CoreField& field, const Decision& decision)
{
    DCHECK(decision.isValid()) << decision.toString();

    static const int checker[6][4] = {
        { 2, 1, 0 },
        { 2, 0 },
        { 0 },
        { 4, 0 },
        { 4, 5, 0 },
        { 4, 5, 6, 0 },
    };

    int checkerIdx = decision.x - 1;
    if (decision.r == 1 && 3 <= decision.x)
        checkerIdx += 1;
    else if (decision.r == 3 && decision.x <= 3)
        checkerIdx -= 1;

    // When decision is valid, this should hold.
    DCHECK(0 <= checkerIdx && checkerIdx < 6) << checkerIdx;

    for (int i = 0; checker[checkerIdx][i] != 0; ++i) {
        if (field.get(checker[checkerIdx][i], 12) != PuyoColor::EMPTY)
            return false;
    }

    return true;
}

bool PuyoController::isReachableFrom(const CoreField& field, const MovingKumipuyoState& mks, const Decision& decision)
{
    return !findKeyStrokeOnlineInternal(field, mks, decision).empty();
}

KeySetSeq PuyoController::findKeyStroke(const CoreField& field, const MovingKumipuyoState& mks, const Decision& decision)
{
    KeySetSeq kss = findKeyStrokeFastpath(field, mks, decision);
    if (!kss.empty())
        return kss;
    return findKeyStrokeOnline(field, mks, decision);
    // return findKeyStrokeByDijkstra(field, mks, decision);
}

typedef MovingKumipuyoState Vertex;

typedef double Weight;
struct Edge {
    Edge() {}
    Edge(const Vertex& src, const Vertex& dest, Weight weight, const KeySet& keySet) :
        src(src), dest(dest), weight(weight), keySet(keySet) {}

    friend bool operator>(const Edge& lhs, const Edge& rhs) {
        if (lhs.weight != rhs.weight) { return lhs.weight > rhs.weight; }
        if (lhs.src != rhs.src) { lhs.src > rhs.src; }
        return lhs.dest > rhs.dest;
    }

    Vertex src;
    Vertex dest;
    Weight weight;
    KeySet keySet;
};

typedef vector<Edge> Edges;
typedef map<Vertex, tuple<Vertex, KeySet, Weight>> Potential;

KeySetSeq PuyoController::findKeyStrokeByDijkstra(const CoreField& field, const MovingKumipuyoState& initialState, const Decision& decision)
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
    static const int KEY_CANDIDATES_WITHOUT_TURN_SIZE = 3;
    static const int KEY_CANDIDATES_SIZE = ARRAY_SIZE(KEY_CANDIDATES);

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

        int size = p.restFramesTurnProhibited > 0 ? KEY_CANDIDATES_WITHOUT_TURN_SIZE : KEY_CANDIDATES_SIZE;
        if (p.grounded)
            size = 0;
        for (int i = 0; i < size; ++i) {
            MovingKumipuyoState mks(p);
            moveKumipuyo(field, KEY_CANDIDATES[i].first, &mks);

            if (pot.count(mks))
                continue;
            // TODO(mayah): This is not correct. We'd like to prefer KeySet() to another key sequence a bit.
            Q.push(Edge(p, mks, curr + KEY_CANDIDATES[i].second, KEY_CANDIDATES[i].first));
        }
    }

    // No way...
    return KeySetSeq();
}

KeySetSeq PuyoController::findKeyStrokeOnline(const CoreField& field, const MovingKumipuyoState& mks, const Decision& decision)
{
    KeySetSeq kss = findKeyStrokeOnlineInternal(field, mks, decision);
    removeRedundantKeySeq(mks.pos, &kss);
    expandButtonDistance(&kss);
    return kss;
}

// returns null if not reachable
KeySetSeq PuyoController::findKeyStrokeOnlineInternal(const CoreField& field, const MovingKumipuyoState& mks, const Decision& decision)
{
    KeySetSeq ret;
    KumipuyoPos current = mks.pos;

    while (true) {
        if (current.x == decision.x && current.r == decision.r) {
            break;
        }

        // for simpicity, direct child-puyo upwards
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
                    if (!field.get(current.x - 1, current.y + 1)) {
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
            if (!field.get(current.x - 1, current.y)) {
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
                    if (!field.get(current.x + 1, current.y)) {
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

namespace {

KeySetSeq findKeyStrokeFastpath1(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        if (field.height(1) <= 11 && field.height(2) <= 11) {
            return KeySetSeq { KeySet(Key::LEFT), KeySet(Key::LEFT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 1:
        if (field.height(1) <= 11 && field.height(2) <= 11) {
            return KeySetSeq { KeySet(Key::LEFT, Key::RIGHT_TURN), KeySet(Key::LEFT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 2:
        if (field.height(1) <= 11 && field.height(2) <= 10) {
            return KeySetSeq {
                KeySet(Key::LEFT, Key::RIGHT_TURN),
                KeySet(Key::LEFT),
                KeySet(Key::DOWN, Key::RIGHT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    default:
        CHECK(false) << "Invalid r: " << decision.r;
    }
}

KeySetSeq findKeyStrokeFastpath2(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        if (field.height(2) <= 11) {
            return KeySetSeq { KeySet(Key::LEFT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 1:
        if (field.height(2) <= 11) {
            return KeySetSeq { KeySet(Key::LEFT, Key::RIGHT_TURN), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 2:
        if (field.height(2) <= 10 && field.height(3) <= 10) {
            return KeySetSeq {
                KeySet(Key::LEFT, Key::RIGHT_TURN),
                KeySet(Key::DOWN),
                KeySet(Key::DOWN, Key::RIGHT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    case 3:
        if (field.height(1) <= 11 && field.height(2) <= 11) {
            return KeySetSeq {
                KeySet(Key::LEFT, Key::LEFT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    default:
        CHECK(false) << "Invalid r: " << decision.r;
    }
}

KeySetSeq findKeyStrokeFastpath3(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        return KeySetSeq { KeySet(Key::DOWN) };
    case 1:
        if (field.height(4) >= 9)
            return KeySetSeq();
        return KeySetSeq { KeySet(Key::DOWN, Key::RIGHT_TURN), KeySet(Key::DOWN) };
    case 2:
        if (field.height(4) <= 6) {
            return KeySetSeq {
                KeySet(Key::DOWN, Key::RIGHT_TURN),
                KeySet(Key::DOWN),
                KeySet(Key::DOWN, Key::RIGHT_TURN),
                KeySet(Key::DOWN),
            };
        }
        if (field.height(2) <= 6) {
            return KeySetSeq {
                KeySet(Key::DOWN, Key::LEFT_TURN),
                KeySet(Key::DOWN),
                KeySet(Key::DOWN, Key::LEFT_TURN),
                KeySet(Key::DOWN),
            };
        }
        return KeySetSeq();
    case 3:
        if (field.height(2) >= 9)
            return KeySetSeq();
        return KeySetSeq {
            KeySet(Key::DOWN, Key::LEFT_TURN),
            KeySet(Key::DOWN)
        };
    default:
        CHECK(false) << "Unknown r: " << decision.r;
    }
}

KeySetSeq findKeyStrokeFastpath4(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        if (field.height(4) <= 11) {
            return KeySetSeq { KeySet(Key::RIGHT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 1:
        if (field.height(4) <= 11 && field.height(5) <= 11) {
            return KeySetSeq { KeySet(Key::RIGHT, Key::RIGHT_TURN), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 2:
        if (field.height(5) <= 9 && field.height(4) <= 9) {
            return KeySetSeq {
                KeySet(Key::RIGHT, Key::LEFT_TURN),
                KeySet(Key::DOWN),
                KeySet(Key::DOWN, Key::LEFT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    case 3:
        if (field.height(2) <= 11 && field.height(3) <= 11 && field.height(4) <= 11) {
            return KeySetSeq {
                KeySet(Key::RIGHT, Key::LEFT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    default:
        CHECK(false) << "Unknown r: " << decision.r;
    }
}

KeySetSeq findKeyStrokeFastpath5(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        if (field.height(4) <= 11 && field.height(5) <= 11) {
            return KeySetSeq { KeySet(Key::RIGHT), KeySet(Key::RIGHT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 1:
        if (field.height(5) <= 11 && field.height(6) <= 11) {
            return KeySetSeq { KeySet(Key::RIGHT, Key::RIGHT_TURN), KeySet(Key::RIGHT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    case 2:
        if (field.height(5) <= 9 && field.height(6) <= 9) {
            return KeySetSeq {
                KeySet(Key::RIGHT, Key::RIGHT_TURN),
                KeySet(Key::RIGHT),
                KeySet(Key::DOWN,Key::RIGHT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    case 3:
        if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11) {
            return KeySetSeq { KeySet(Key::RIGHT, Key::LEFT_TURN), KeySet(Key::RIGHT), KeySet(Key::DOWN) };
        }
        return KeySetSeq();
    default:
        CHECK(false) << "Unknown r: " << decision.r;
    }
    return KeySetSeq();
}

KeySetSeq findKeyStrokeFastpath6(const CoreField& field, const Decision& decision)
{
    switch (decision.r) {
    case 0:
        if (field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11 ) {
            return KeySetSeq {
                KeySet(Key::RIGHT),
                KeySet(Key::RIGHT),
                KeySet(Key::RIGHT),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    case 2:
        if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 9 && field.height(6) <= 9) {
            return KeySetSeq {
                KeySet(Key::RIGHT, Key::LEFT_TURN),
                KeySet(Key::RIGHT),
                KeySet(Key::RIGHT, Key::LEFT_TURN),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    case 3:
        if (field.height(2) <= 11 && field.height(4) <= 11 && field.height(5) <= 11 && field.height(6) <= 11) {
            return KeySetSeq {
                KeySet(Key::RIGHT, Key::LEFT_TURN),
                KeySet(Key::RIGHT),
                KeySet(Key::RIGHT),
                KeySet(Key::DOWN)
            };
        }
        return KeySetSeq();
    default:
        CHECK(false) << "Unknown r: " << decision.r;
    }
    return KeySetSeq();
}

}

KeySetSeq PuyoController::findKeyStrokeFastpath(const CoreField& field, const MovingKumipuyoState& mks, const Decision& decision)
{
    // fastpath only works when pos is located at initial position.
    if (!(mks.pos.x == 3 && mks.pos.y == 12 && mks.pos.r == 0))
        return KeySetSeq();

    switch (decision.x) {
    case 1: return findKeyStrokeFastpath1(field, decision);
    case 2: return findKeyStrokeFastpath2(field, decision);
    case 3: return findKeyStrokeFastpath3(field, decision);
    case 4: return findKeyStrokeFastpath4(field, decision);
    case 5: return findKeyStrokeFastpath5(field, decision);
    case 6: return findKeyStrokeFastpath6(field, decision);
    default:
        CHECK(false) << "Unknown x: " << decision.x;
        return KeySetSeq();
    }
}
