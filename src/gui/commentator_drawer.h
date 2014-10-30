#ifndef GUI_COMMENTATOR_DRAWER_H_
#define GUI_COMMENTATOR_DRAWER_H_

#include <mutex>

#include "core/server/commentator.h"
#include "gui/drawer.h"

class Screen;

class CommentatorDrawer : public Drawer, public CommentatorObserver {
public:
    CommentatorDrawer();
    virtual ~CommentatorDrawer();

    virtual void onCommentatorResultUpdate(const CommentatorResult&) override;
    virtual void draw(Screen*) override;

private:
    void drawMainChain(Screen*, const CommentatorResult&) const;
    void drawCommentSurface(Screen*, const CommentatorResult&, int playerId) const;

    std::mutex mu_;
    CommentatorResult result_;
};

#endif
