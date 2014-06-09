#ifndef GUI_BOUNDING_BOX_DRAWER_H_
#define GUI_BOUNDING_BOX_DRAWER_H_

#include "base/base.h"
#include "gui/drawer.h"

class BoundingBoxDrawer : public Drawer {
public:
    virtual ~BoundingBoxDrawer() {}

    virtual void draw(Screen*) OVERRIDE;
};

#endif
