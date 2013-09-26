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

    void simulate(BasicRensaResult& rensaResult, int additionalChains = 0);
    void simulateAndTrack(BasicRensaResult& rensaResult, RensaTrackResult& trackResult, int additionalChains = 0);
    
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

    // Simulates chains. Returns chains, score, and frames before finishing the chain.
    template<typename Tracker>
    void simulateWithTracker(BasicRensaResult&, Tracker&);

    // Vanishes puyos., and adds score. The argument "chains" is used to calculate score.
    template<typename Tracker>
    bool vanish(int nthChain, int* score, int minHeights[], Tracker&);
    template<typename Tracker>
    void eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Tracker&);
    template<typename Tracker>
    int dropAfterVanish(int minHeights[], Tracker&);

    Position* checkCell(PuyoColor, FieldBitField& checked, Position* writeHead, int x, int y) const;

    Position* fillSameColorPosition(int x, int y, PuyoColor, Position* positionQueueHead, FieldBitField& checked) const;

    Puyo m_field[MAP_WIDTH][MAP_HEIGHT];
    byte m_heights[MAP_WIDTH];
};

#endif  // __BASIC_FIELD_H_
