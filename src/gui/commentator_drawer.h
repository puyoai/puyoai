#ifndef GUI_COMMENTATOR_DRAWER_H_
#define GUI_COMMENTATOR_DRAWER_H_

#include "core/server/commentator.h"
#include "gui/drawer.h"

class Screen;

class CommentatorDrawer : public Drawer {
public:
    explicit CommentatorDrawer(const Commentator*);
    virtual ~CommentatorDrawer();

    virtual void draw(Screen*) override;

private:
    void drawMainChain(Screen*, const CommentatorResult&) const;
    void drawCommentSurface(Screen*, const CommentatorResult&, int playerId) const;

    const Commentator* commentator_;
};

#endif
