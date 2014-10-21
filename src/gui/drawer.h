#ifndef GUI_DRAWER_H_
#define GUI_DRAWER_H_

class Screen;

class Drawer {
public:
    virtual ~Drawer() {}
    virtual void onInit() {}
    virtual void draw(Screen*) = 0;
};

#endif
