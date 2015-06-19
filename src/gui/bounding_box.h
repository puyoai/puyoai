#ifndef GUI_BOUNDING_BOX_H_
#define GUI_BOUNDING_BOX_H_

#include <map>

#include "base/noncopyable.h"
#include "core/next_puyo.h"
#include "gui/box.h"

class BoundingBox : private noncopyable {
public:
    enum class Region {
        LEVEL_SELECT_1P,
        LEVEL_SELECT_2P,
        GAME_FINISHED,
    };

    static Box boxForDraw(int pi, int x, int y);
    static Box boxForDraw(int pi, NextPuyoPosition);
    static Box boxForDraw(Region region);

    static Box boxForAnalysis(int pi, int x, int y);
    static Box boxForAnalysis(int pi, NextPuyoPosition);
    static Box boxForAnalysis(Region region);
private:
    BoundingBox() = delete;
    ~BoundingBox() = delete;
};

#endif
