#include "duel/cui_win.h"

#include <windows.h>
#undef ERROR

#include <glog/logging.h>

#include "core/field_constant.h"

using std::string;
using std::cout;

CuiWin::CuiWin()
{
    console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &consoleInfo);
}

void CuiWin::clear()
{
    cls();
    printPuyoCache_.clear();
    printTextCache_.clear();
}

void CuiWin::newGameWillStart()
{
    COORD coord = {0, FieldConstant::MAP_HEIGHT + 3};
    DWORD unused;
    FillConsoleOutputCharacter(console, static_cast<TCHAR>(' '), consoleInfo.dwSize.X, coord, &unused);
    coord.Y = FieldConstant::MAP_HEIGHT + 4;
    FillConsoleOutputCharacter(console, static_cast<TCHAR>(' '), consoleInfo.dwSize.X, coord, &unused);

    printPuyoCache_.clear();
    printTextCache_.clear();
}

void CuiWin::setCursor(const Location& location)
{
    setCursor(location.x, location.y);
}

void CuiWin::setCursor(int x, int y)
{
    COORD pos = {x - 1, y - 1};
    SetConsoleCursorPosition(console, pos);
}

void CuiWin::setColor(PuyoColor color)
{
    WORD attribute = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
    switch(color) {
    case PuyoColor::RED:
        attribute = BACKGROUND_RED | BACKGROUND_INTENSITY;
        break;
    case PuyoColor::BLUE:
        attribute = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
        break;
    case PuyoColor::GREEN:
        attribute = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
        break;
    case PuyoColor::YELLOW:
        attribute = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
        break;
    default:
        attribute = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
        break;
    }
    SetConsoleTextAttribute(console, attribute | FOREGROUND_INTENSITY);
}

void CuiWin::cls()
{
    COORD coord = {0, 0};
    DWORD unused;

    int numChars = consoleInfo.dwSize.X * consoleInfo.dwSize.Y;
    FillConsoleOutputCharacter(console, static_cast<TCHAR>(' '), numChars, coord, &unused);
    setCursor(1, 1);
 }
