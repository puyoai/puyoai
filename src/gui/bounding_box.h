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

    void setGenerator(double offsetX, double offsetY, double bbWidth, double bbHeight);
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

    double offsetX_;
    double offsetY_;
    double bbWidth_;
    double bbHeight_;

    std::map<Region, Box> regionBox_;
};

#endif
