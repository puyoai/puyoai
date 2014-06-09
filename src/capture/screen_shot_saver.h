#ifndef CAPTURE_SCREEN_SHOT_SAVER_H_
#define CAPTURE_SCREEN_SHOT_SAVER_H_

#include <stdint.h>
#include "base/base.h"
#include "gui/drawer.h"

class Screen;

// Behaves as a drawer, and save screen shot.
class ScreenShotSaver : public Drawer {
public:
    ScreenShotSaver() : lastFrameId_(0) {}
    virtual ~ScreenShotSaver() {}

    // If true, we draw the frameId and save the screenshot with it.
    void setDrawsFrameId(bool flag) { drawsFrameId_ = flag; }

    virtual void draw(Screen*) OVERRIDE;

private:
    uintptr_t lastFrameId_;
    bool drawsFrameId_ = false;
};

#endif
