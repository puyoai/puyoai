#ifndef GUI_BOUNDING_BOX_H_
#define GUI_BOUNDING_BOX_H_

#include <map>

#include "base/noncopyable.h"
#include "core/next_puyo.h"
#include "gui/box.h"

class BoundingBox : private noncopyable {
public:
    enum class Region {
        LEVEL_SELECT,
        GAME_FINISHED,
    };

    static BoundingBox& instance();

    void setGenerator(int offsetX, int offsetY, int bbWidth, int bbHeight);
    Box get(int pi, int x, int y) const;
    Box get(int pi, NextPuyoPosition n) const;

    void setRegion(Region region, const Box& box) { regionBox_[region] = box; }
    Box getBy(Region region) const {
        auto it = regionBox_.find(region);
        if (it != regionBox_.end())
            return it->second;

        return Box(0, 0, 0, 0);
    }

private:
    BoundingBox();
    ~BoundingBox();

    int offsetX_;
    int offsetY_;
    int bbWidth_;
    int bbHeight_;

    std::map<Region, Box> regionBox_;
};

#endif
