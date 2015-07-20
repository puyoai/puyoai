#ifndef GUI_FRAME_NUBMER_DRAWER_H_
#define GUI_FRAME_NUBMER_DRAWER_H_

#include <SDL.h>

#include "base/base.h"
#include "gui/drawer.h"

class FrameNumberDrawer : public Drawer {
public:
    FrameNumberDrawer() {}
    ~FrameNumberDrawer() override {}

    void draw(Screen*) override;

private:
    size_t frameNumber_ = 0;
};

#endif
