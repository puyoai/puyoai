#ifndef CORE_CORE_FIELD_H_
#define CORE_CORE_FIELD_H_

#include <glog/logging.h>

#include <algorithm>
#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

#include "base/base.h"
#include "core/bit_field.h"
#include "core/decision.h"
#include "core/field_checker.h"
#include "core/field_constant.h"
#include "core/kumipuyo_pos.h"
#include "core/puyo_color.h"
#include "core/plain_field.h"
#include "core/rensa_result.h"
#include "core/rensa_track_result.h"
#include "core/rensa_tracker.h"

class ColumnPuyoList;
class Kumipuyo;
struct Position;

// CoreField represents a field. Without strong reason, this class should be used for
// field implementation.
// TODO(mayah): All puyos in CoreField should be grounded (14th row is exception).
// For the field that might contain puyo in the air, it would be better to make another class.
class CoreField : public FieldConstant {
public:
    CoreField() : heights_{} {}
    explicit CoreField(const std::string& url);
    explicit CoreField(const PlainField&);
    CoreField(const CoreField&) = default;

    static CoreField fromPlainFieldWithDrop(const PlainField& pf) {
        PlainField copied(pf);
        copied.drop();
        return CoreField(copied);
    }

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return field_.color(x, y); }

    // Returns true if puyo on (x, y) is |c|.
    bool isColor(int x, int y, PuyoColor c) const { return field_.isColor(x, y, c); }
    // Returns true if puyo on (x, y) is empty.
    bool isEmpty(int x, int y) const { return field_.isEmpty(x, y); }
    // Returns true if puyo on (x, y) is a normal color.
    bool isNormalColor(int x, int y) const { return field_.isNormalColor(x, y); }

    // Returns the height of the specified column.
    int height(int x) const { return heights_[x]; }

    const BitField& bitField() const { return field_; }
    PlainField toPlainField() const;

    // TODO(mayah): Remove plainField(). Use toPlainField() everywhere.
    PlainField plainField() const { return toPlainField(); }

    // ----------------------------------------------------------------------
    // field utilities

    // Returns true if the field does not have any puyo. Valid only all puyos are dropped.
    // TODO(mayah): Remove isZenkeshiPrecise() from CoreField when all the puyo is not in the air.
    bool isZenkeshi() const;

    // Counts the number of color puyos.
    int countColorPuyos() const;
    // Counts the all puyos (including ojama).
    int countPuyos() const;
    // Returns the number of puyos connected to (x, y).
    // Actually you can use this if color(x, y) is EMPTY or OJAMA.
    int countConnectedPuyos(int x, int y) const { return field_.countConnectedPuyos(x, y); }
    // Same as countConnectedPuyos(x, y), but with checking using |checked|.
    int countConnectedPuyos(int x, int y, FieldBits* checked) const { return field_.countConnectedPuyos(x, y, checked); }
    // Same as countConnectedPuyos(x, y).
    // If # of connected puyos is >= 4, the result is any value >= 4.
    // For example, if the actual number of connected is 6, result is 4, 5, or 6.
    // This is faster than countConnectedPuyos, so this will be useful when checking
    // puyo is vanished or not.
    int countConnectedPuyosMax4(int x, int y) const { return field_.countConnectedPuyosMax4(x, y); }
    // Returns true if color(x, y) is connected in some direction.
    bool isConnectedPuyo(int x, int y) const { return field_.isConnectedPuyo(x, y); }
    // Returns true if there is an empty neighbor of (x, y).
    bool hasEmptyNeighbor(int x, int y) const { return field_.hasEmptyNeighbor(x, y); }
    // Returns the number of empty unreachable spaces.
    int countUnreachableSpaces() const;

    void countConnection(int* count2, int* count3) const { return field_.countConnection(count2, count3); }

    // ----------------------------------------------------------------------
    // field manipulation

    // Drop kumipuyo with decision.
    bool dropKumipuyo(const Decision&, const Kumipuyo&);

    // Remove puyos.
    void undoKumipuyo(const Decision&);

    // Returns #frame to drop the next KumiPuyo with decision. This function does not drop the puyo.
    int framesToDropNext(const Decision&) const;

    // Returns true if |decision| will cause chigiri.
    bool isChigiriDecision(const Decision&) const;

    // Fall ojama puyos |lines| lines.
    // Returns the frame to fall ojama.
    int fallOjama(int lines);

    // Places a puyo on the top of column |x|.
    // Returns true if succeeded. False if failed. When false is returned, field will not change.
    bool dropPuyoOn(int x, PuyoColor pc) { return dropPuyoOnWithMaxHeight(x, pc, 13); }
    bool dropPuyoOnWithMaxHeight(int x, PuyoColor, int maxHeight);

    // Drop all puyos in |cpl|. If failed, false will be returned. In that case, the CoreField
    // might be corrupted, so you cannot use this CoreField.
    bool dropPuyoList(const ColumnPuyoList& cpl) { return dropPuyoListWithMaxHeight(cpl, 13); }
    bool dropPuyoListWithMaxHeight(const ColumnPuyoList&, int maxHeight);

    // Removes the puyo from top of column |x|.
    // If there is no puyo on column |x|, behavior is undefined.
    void removePuyoFrom(int x);
    // Removes |n| puyos from top of column |x|.
    // If there are not |n| puyos on column |x|, behavior is undefined.
    void removePuyoFrom(int x, int n);
    // Removes puyo list.
    void remove(const ColumnPuyoList&);

    // ----------------------------------------------------------------------
    // simulation

    // SimulationContext can be used when we continue simulation from the intermediate points.
    typedef BitField::SimulationContext SimulationContext;

    // Inserts positions whose puyo color is the same as |c|, and connected to (x, y).
    // The checked cells will be marked in |checked|.
    // PositionQueueHead should have enough capacity.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBits* checked) const
    {
        return field_.fillSameColorPosition(x, y, c, positionQueueHead, checked);
    }

    // Fills the positions where puyo is vanished in the 1-rensa.
    // Returns the length of the filled positions. The max length should be 72.
    // So, |Position*| must have 72 Position spaces.
    int fillErasingPuyoPositions(Position*) const;
    // TODO(mayah): Remove this.
    std::vector<Position> erasingPuyoPositions() const;

    // TODO(mayah): Remove this.
    bool rensaWillOccurWhenLastDecisionIs(const Decision&) const;
    // TODO(mayah): Remove this.
    bool rensaWillOccurWithContext(const SimulationContext&) const;
    bool rensaWillOccur() const { return field_.rensaWillOccur(); }

    // Simulates chains. Returns RensaResult.
    RensaResult simulate(int initialChain = 1);
    // Simulates chains with SimulationContext.
    RensaResult simulate(SimulationContext*);
    // Simulates chains with Tracker. Tracker can track various rensa information.
    // Several trackers are defined in core/rensa_trackers.h. You can define your own Tracker.
    template<typename Tracker>
    RensaResult simulate(Tracker*);
    // Simualtes chains with SimulationContext and Tracker.
    template<typename Tracker>
    RensaResult simulate(SimulationContext*, Tracker*) NOINLINE_UNLESS_RELEASE;

    // Vanishes the connected puyos, and drop the puyos in the air. Score will be returned.
    RensaStepResult vanishDrop();
    RensaStepResult vanishDrop(SimulationContext*);
    // Vanishes the connected puyos with Tracker.
    template<typename Tracker>
    RensaStepResult vanishDrop(Tracker*);
    template<typename Tracker>
    RensaStepResult vanishDrop(SimulationContext*, Tracker*) NOINLINE_UNLESS_RELEASE;

    // ----------------------------------------------------------------------
    // utility methods

    std::string toDebugString() const;

    friend bool operator==(const CoreField&, const CoreField&);
    friend bool operator!=(const CoreField&, const CoreField&);
    friend std::ostream& operator<<(std::ostream& os, const CoreField& cf) { return (os << cf.toDebugString()); }

public:
    // --- These methods should be carefully used.

    // TODO(mayah): Remove this.
    void setPuyoAndHeight(int x, int y, PuyoColor c)
    {
        unsafeSet(x, y, c);

        // Recalculate height.
        heights_[x] = 0;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                heights_[x] = y;
        }
    }

private:
    void unsafeSet(int x, int y, PuyoColor c) { field_.setColor(x, y, c); }

    BitField field_;
    alignas(16) std::uint16_t heights_[MAP_WIDTH];
};

#include "core/core_field_inl.h"

#endif
