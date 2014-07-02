#ifndef CORE_CORE_FIELD_H_
#define CORE_CORE_FIELD_H_

#include <stdint.h>

#include "base/base.h"
#include "core/puyo_color.h"
#include "core/plain_field.h"

class Decision;
class FieldBitField;
class Kumipuyo;
struct Position;
class RensaTrackResult;
struct RensaResult;

class CoreField : public PlainField {
public:
    CoreField();
    CoreField(const std::string& url);
    CoreField(const CoreField&);
    CoreField(const PlainField&);

    void clear();

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return get(x, y); }

    // Returns the height of the specified column.
    int height(int x) const { return heights_[x]; }

    // ----------------------------------------------------------------------
    // field utilities

    // Returns true if the field does not have any puyo.
    bool isZenkeshi() const;

    // Counts the number of color puyos.
    int countColorPuyos() const;
    // Counts the all puyos (including ojama).
    int countPuyos() const;
    // Returns the number of puyos connected to (x, y).
    // Actually you can use this if color(x, y) is EMPTY or OJAMA.
    int countConnectedPuyos(int x, int y) const;
    // Same as countConnectedPuyos(x, y), but with checking using |checked|.
    int countConnectedPuyos(int x, int y, FieldBitField* checked) const;

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

    bool rensaWillOccurWhenLastDecisionIs(const Decision&) const;

    // Simulates rensa.
    RensaResult simulate(int initialChain = 1);

    // Simulates rensa, but the last decision is specified. This will be faster then simulate().
    RensaResult simulateWhenLastDecisionIs(const Decision&);

    // Simulates rensa with specifying the original heights.
    // Note that minHeights will be modified after function return.
    RensaResult simulateWithMinHeights(int minHeights[MAP_WIDTH]);

    // Simulates rensa with tracking.
    RensaResult simulateAndTrack(RensaTrackResult* trackResult, int initialChain = 1);

    RensaResult simulateAndTrackWithMinHeights(RensaTrackResult* trackResult, int minHeights[MAP_WIDTH]);

    // Vanish the connected puyos. Score will be returned.
    int vanishOnly(int currentChain);

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
    void recalcHeightOn(int x) {
        heights_[x] = 0;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                heights_[x] = y;
        }
    }
    void setPuyoAndHeight(int x, int y, PuyoColor c) {
        unsafeSet(x, y, c);
        // TODO(mayah): We should be able to skip some calculation of this recalc.
        recalcHeightOn(x);
    }

protected:
    // Simulates chains. Returns RensaResult.
    template<typename Tracker>
    RensaResult simulateWithTracker(int initialChain, int minHeights[], Tracker*);

    // Vanishes connected puyos and returns score. If score is 0, no puyos are vanished.
    template<typename Tracker>
    int vanish(int nthChain, int minHeights[], Tracker*);

    // Erases puyos in queue.
    template<typename Tracker>
    void eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Tracker*);

    // Drops puyos in the air after vanishment.
    template<typename Tracker>
    int dropAfterVanish(int minHeights[], Tracker*);

    // Inserts positions whose puyo color is the same as |c|, and connected to (x, y).
    // The checked cells will be marked in |checked|.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBitField* checked) const;

    uint8_t heights_[MAP_WIDTH];
};

#endif
