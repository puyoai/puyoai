#ifndef CORE_CORE_FIELD_H_
#define CORE_CORE_FIELD_H_

#include <cstdint>
#include <initializer_list>

#include "base/base.h"
#include "core/decision.h"
#include "core/kumipuyo_pos.h"
#include "core/puyo_color.h"
#include "core/plain_field.h"
#include "core/rensa_result.h"

class FieldBitField;
class Kumipuyo;
struct Position;
class RensaTrackResult;

class CoreField : public PlainField {
public:
    CoreField();
    explicit CoreField(const std::string& url);
    explicit CoreField(const PlainField&);
    CoreField(const CoreField&) = default;

    void clear();

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return get(x, y); }

    // Returns the height of the specified column.
    int height(int x) const { return heights_[x]; }

    // ----------------------------------------------------------------------
    // field utilities

    // Returns true if the field does not have any puyo. Valid only all puyos are dropped.
    bool isZenkeshi() const;
    // Retruns true if the field does not have any puyo. This will return valid value
    // when some puyos are in the air.
    bool isZenkeshiPrecise() const;

    // Counts the number of color puyos.
    int countColorPuyos() const;
    // Counts the all puyos (including ojama).
    int countPuyos() const;
    // Returns the number of puyos connected to (x, y).
    // Actually you can use this if color(x, y) is EMPTY or OJAMA.
    int countConnectedPuyos(int x, int y) const;
    // Same as countConnectedPuyos(x, y), but with checking using |checked|.
    int countConnectedPuyos(int x, int y, FieldBitField* checked) const;
    // Same as countConnectedPuyos(x, y). But you can call this only when the number of connected puyos <= 3.
    int countConnectedPuyosMax4(int x, int y) const;
    // Returns true if color(x, y) is connected in some direction.
    bool isConnectedPuyo(int x, int y) const;
    // Returns true if neighbor is empty.
    bool hasEmptyNeighbor(int x, int y) const;

    // Inserts positions whose puyo color is the same as |c|, and connected to (x, y).
    // The checked cells will be marked in |checked|.
    // PositionQueueHead should have enough capacity.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBitField* checked) const;

    // ----------------------------------------------------------------------
    // field manipulation

    // Drop kumipuyo with decision.
    bool dropKumipuyo(const Decision&, const Kumipuyo&);

    // Remove puyos.
    void undoKumipuyo(const Decision&);

    // Returns the position to drop.
    KumipuyoPos dropPosition(const Decision&) const;

    // Returns #frame to drop the next KumiPuyo with decision. This function does not drop the puyo.
    int framesToDropNext(const Decision&) const;

    // Returns true if |decision| will cause chigiri.
    bool isChigiriDecision(const Decision&) const;

    // Places a puyo on the top of column |x|.
    // Returns true if succeeded. False if failed. When false is returned, field will not change.
    bool dropPuyoOn(int x, PuyoColor, bool isAxis = false);

    // Removes the puyo from top of column |x|. If there is no puyo on column |x|, nothing will happen.
    void removeTopPuyoFrom(int x) {
        if (height(x) > 0)
            unsafeSet(x, heights_[x]--, PuyoColor::EMPTY);
    }

    // Drops all puyos if some puyos are in the air.
    void forceDrop();

    // ----------------------------------------------------------------------
    // simulation

    // SimulationContext can be used when we continue simulation from the intermediate points.
    struct SimulationContext {
        explicit SimulationContext(int currentChain) : currentChain(currentChain) {}
        SimulationContext(int currentChain, std::initializer_list<int> list) :
            currentChain(currentChain)
        {
            DCHECK_EQ(list.size(), 8UL) << list.size();
            std::copy(list.begin(), list.end(), minHeights);
        }

        static SimulationContext fromLastDecision(const CoreField&, const Decision& lastDecision);
        static SimulationContext fromField(const CoreField&);

        int currentChain = 1;
        int minHeights[FieldConstant::MAP_WIDTH] = { 1, 1, 1, 1, 1, 1, 1, 1 };

    private:
        SimulationContext(int currentChain, const CoreField& cf) :
            currentChain(currentChain),
            minHeights {
                1, cf.height(1) + 1, cf.height(2) + 1, cf.height(3) + 1,
                cf.height(4) + 1, cf.height(5) + 1, cf.height(6) + 1, 1
            }
        {
        }
    };

    bool rensaWillOccurWhenLastDecisionIs(const Decision&) const;
    bool rensaWillOccurWithContext(SimulationContext*) const;

    // Simulates rensa.
    // When trackResult is passed, RensaTrackResult will be fulfilled.
    RensaResult simulate(int initialChain = 1,
                         RensaTrackResult* rensaTrackResult = nullptr,
                         RensaCoefResult* rensaCoefResult = nullptr,
                         RensaVanishingPositionResult* rensaVanishingPositionResult = nullptr);
    RensaResult simulate(RensaCoefResult* rensaCoefResult) { return simulate(1, nullptr, rensaCoefResult, nullptr); }
    RensaResult simulate(RensaTrackResult* rensaTrackResult) { return simulate(1, rensaTrackResult, nullptr, nullptr); }
    RensaResult simulate(RensaVanishingPositionResult* rensaVanishingPositionResult) { return simulate(1, nullptr, nullptr, rensaVanishingPositionResult); }

    // Simulates rensa with specifying the context.
    RensaResult simulateWithContext(SimulationContext*);
    RensaResult simulateWithContext(SimulationContext*, RensaTrackResult*);
    RensaResult simulateWithContext(SimulationContext*, RensaCoefResult*);
    RensaResult simulateWithContext(SimulationContext*, RensaVanishingPositionResult*);

    // Vanishes the connected puyos. Score will be returned.
    int vanishOnly(int currentChain);

    // Vanishes the connected puyos, and drop the puyos in the air. Score will be returned.
    int vanishDrop(SimulationContext*);

    // ----------------------------------------------------------------------
    // utility methods

    std::string toDebugString() const;

    friend bool operator==(const CoreField&, const CoreField&);
    friend bool operator!=(const CoreField&, const CoreField&);

public:
    // --- These methods should be carefully used.
    // Sets puyo on arbitrary position. After setColor, you have to call recalcHeightOn.
    // Otherwise, the field will be broken.
    // Recalculates height on column |x|.
    void recalcHeightOn(int x)
    {
        heights_[x] = 0;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                heights_[x] = y;
        }
    }

    void setPuyoAndHeight(int x, int y, PuyoColor c)
    {
        unsafeSet(x, y, c);
        // TODO(mayah): We should be able to skip some calculation of this recalc.
        recalcHeightOn(x);
    }

protected:
    // Simulates chains. Returns RensaResult.
    template<typename Tracker>
    RensaResult simulateWithTracker(SimulationContext*, Tracker*);

    // Vanishes connected puyos and returns score. If score is 0, no puyos are vanished.
    template<typename Tracker>
    int vanish(SimulationContext*, Tracker*);

    // Erases puyos in queue.
    template<typename Tracker>
    void eraseQueuedPuyos(SimulationContext*, Position* eraseQueue, Position* eraseQueueHead, Tracker*);

    // Drops puyos in the air after vanishment.
    // Returns the max drop height.
    template<typename Tracker>
    int dropAfterVanish(SimulationContext*, Tracker*);

    int heights_[MAP_WIDTH];
};

// static
inline CoreField::SimulationContext
CoreField::SimulationContext::fromField(const CoreField& cf)
{
    return CoreField::SimulationContext(1, cf);
}

// static
inline CoreField::SimulationContext
CoreField::SimulationContext::fromLastDecision(const CoreField& cf,
                                               const Decision& decision)
{
    CoreField::SimulationContext context = fromField(cf);
    context.minHeights[decision.axisX()]--;
    context.minHeights[decision.childX()]--;
    return context;
}

#endif
