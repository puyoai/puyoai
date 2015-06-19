#include "capture/real_color_field.h"

RealColorField::RealColorField()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            set(x, y, RealColor::RC_EMPTY);
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        set(x, 0, RealColor::RC_WALL);
        set(x, MAP_HEIGHT - 1, RealColor::RC_WALL);
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        set(0, y, RealColor::RC_WALL);
        set(MAP_WIDTH - 1, y, RealColor::RC_WALL);
    }
}

RealColorField::RealColorField(const std::string& url) :
    RealColorField()
{
    int counter = 0;
    for (int i = url.length() - 1; i >= 0; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        RealColor rc = toRealColor(url[i]);
        setColor(x, y, rc);
        counter++;
    }
}
