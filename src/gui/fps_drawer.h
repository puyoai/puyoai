#ifndef GUI_FPS_DRAWER_H_
#define GUI_FPS_DRAWER_H_

#include <SDL.h>

#include "base/base.h"
#include "gui/drawer.h"

class FPSDrawer : public Drawer {
public:
    FPSDrawer();
    virtual ~FPSDrawer();
    virtual void draw(Screen*) override;

private:
    size_t frames_;
    Uint32 ticks_[30];
};

#endif
