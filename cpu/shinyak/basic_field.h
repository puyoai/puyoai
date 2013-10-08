#ifndef __BASIC_FIELD_H_
#define __BASIC_FIELD_H_

#include "puyo.h"

class Decision;
class FieldBitField;
class RensaTrackResult;
struct BasicRensaResult;
struct Position;

class BasicField {
public:
    static const int WIDTH = 6;
    static const int HEIGHT = 12;
    static const int MAP_WIDTH = 1 + WIDTH + 1;
    static const int MAP_HEIGHT = 1 + HEIGHT + 3;

    BasicField();
    BasicField(const std::string& url);
    BasicField(const BasicField&);

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return static_cast<PuyoColor>(m_field[x][y]); }

    // Returns the height of the specified column.
    int height(int x) const { return m_heights[x]; }

    // Drop kumipuyo with decision.
    void dropKumiPuyo(const Decision&, const KumiPuyo&);

    // Returns #frame to drop the next KumiPuyo with decision. This function does not drop the puyo.
    int framesToDropNext(const Decision&) const;

    // Places a puyo on column |x|.
    void dropPuyoOn(int x, PuyoColor);

    // Removes the puyo from top of column |x|. If there is no puyo on column |x|, nothing will happen.
    void removeTopPuyoFrom(int x) {
        if (height(x) > 0)
            m_field[x][m_heights[x]--] = EMPTY;
    }

    // Drops all puyos if some puyos are in the air.
    void forceDrop();

    BasicRensaResult simulate(int initialChain = 1);
    BasicRensaResult simulateAndTrack(RensaTrackResult* trackResult, int initialChain = 1);
    
    // Normal print for debugging purpose.
    std::string debugOutput() const;

protected:
    // Crears every data this class has.
    void initialize();

    // --- These 2 methods should be carefully used.
    // Sets puyo on arbitrary position. After setColor, you have to call recalcHeightOn.
    // Otherwise, the field will be broken.
    void setPuyo(int x, int y, PuyoColor c) { m_field[x][y] = static_cast<Puyo>(c); }
    // Recalculates height on column |x|.
    void recalcHeightOn(int x) {
        m_heights[x] = 0;
        for (int y = 1; color(x, y) != EMPTY; ++y)
            m_heights[x] = y;
    }    

    // Simulates chains. Returns BasicRensaResult.
    template<typename Tracker>
    BasicRensaResult simulateWithTracker(int initialChain, Tracker*);

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

    Puyo m_field[MAP_WIDTH][MAP_HEIGHT];
    byte m_heights[MAP_WIDTH];
};

#endif  // __BASIC_FIELD_H_
