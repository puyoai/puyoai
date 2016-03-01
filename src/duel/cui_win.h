#ifndef DUEL_CUI_WIN_H_
#define DUEL_CUI_WIN_H_

#include <windows.h>
#undef ERROR

#include <iostream>
#include <string>
#include <unordered_map>

#include "duel/cui.h"

class CuiWin : public Cui {
public:
    CuiWin();
    virtual ~CuiWin() {}

    virtual void clear() override;
    virtual void newGameWillStart() override;

protected:
    virtual void setCursor(const Location& location) override;
    virtual void setCursor(int x, int y) override;
    virtual void setColor(PuyoColor c) override;

private:
    void cls();

    HANDLE console;
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
};

#endif  // DUEL_CUI_WIN_H_
