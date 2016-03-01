#include "duel/cui.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "core/server/game_state.h"

using std::string;
using std::cout;

namespace {

const int kMaxMessageLines = 6;

const char C_RED[] = "\x1b[41m";
const char C_BLUE[] = "\x1b[44m";
const char C_GREEN[] = "\x1b[42m";
const char C_YELLOW[] = "\x1b[43m";
const char C_BLACK[] = "\x1b[49m";

} // namespace

void Cui::clear()
{
    // Clear all characters on screen
    cout << "\x1b[2J";
    flush();
    printPuyoCache_.clear();
    printTextCache_.clear();
}

void Cui::flush()
{
    cout << std::flush;
}

void Cui::newGameWillStart()
{
    setCursor(1, 1 + FieldConstant::MAP_HEIGHT + 3);
    // Clear all characters in the line.
    cout << "\x1B[0K";
    setCursor(1, 1 + FieldConstant::MAP_HEIGHT + 4);
    // Clear all characters in the line.
    cout << "\x1B[0K";
    flush();
    printPuyoCache_.clear();
    printTextCache_.clear();
}

void Cui::onUpdate(const GameState& gameState)
{
    print(0, gameState.playerGameState(0));
    print(1, gameState.playerGameState(1));
    flush();
}

void Cui::printField(int playerId, const PlayerGameState& pgs)
{
    const Kumipuyo& kumipuyo = pgs.kumipuyoSeq.front();
    const KumipuyoPos& kumipuyoPos = pgs.kumipuyoPos;

    for (int y = 0; y < FieldConstant::MAP_HEIGHT; y++) {
        for (int x = 0; x < FieldConstant::MAP_WIDTH; x++) {
            PuyoColor color = pgs.field.color(x, y);
            if (pgs.playable) {
                if (x == kumipuyoPos.axisX() && y == kumipuyoPos.axisY())
                    color = kumipuyo.axis;
                if (x == kumipuyoPos.childX() && y == kumipuyoPos.childY())
                    color = kumipuyo.child;
            }

            printPuyo(locate(playerId, x * 2, FieldConstant::MAP_HEIGHT - y),
                      puyoText(color, y));
        }
    }
    setColor(PuyoColor::EMPTY);
}

void Cui::printNextPuyo(int playerId, const PlayerGameState& pgs)
{
    static const NextPuyoPosition npp[] = {
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
    };

    // Next puyo info
    for (int i = 0; i < 4; ++i) {
        printPuyo(locate(playerId, 9 * 2, 3 + i + (i / 2)),
                  puyoText(pgs.kumipuyoSeq.color(npp[i])));
    }
    setColor(PuyoColor::EMPTY);
}

void Cui::printMessage(int playerId, const std::string& message)
{
    if (message.empty())
        return;

    int baseRow = 1 + FieldConstant::MAP_HEIGHT + 3 + playerId * kMaxMessageLines;

    // Reset location of the cursor.
    int i = 0;
    for (size_t from = 0; from < message.size(); ++i) {
        size_t p = message.find(from, '\n');
        if (p == string::npos || i == kMaxMessageLines - 1)
            p = message.size();
        printText(locate(1, baseRow + i), message.substr(from, p - from));
        from = p + 1;
    }
    for (; i < kMaxMessageLines; ++i) {
        printText(locate(1, baseRow + i), "");
    }
}

void Cui::print(int playerId, const PlayerGameState& pgs)
{
    printField(playerId, pgs);
    printOjamaPuyo(playerId, pgs);
    printNextPuyo(playerId, pgs);
    printMessage(playerId, pgs.message);
    printScore(playerId, pgs.score);

    setCursor(1, FieldConstant::MAP_HEIGHT + 3);
}

void Cui::printScore(int playerId, int score)
{
    std::ostringstream ss;
    ss << std::setw(10) << score;
    printText(locate(playerId, 0, FieldConstant::MAP_HEIGHT + 1),
              ss.str());
}

void Cui::printOjamaPuyo(int playerId, const PlayerGameState& pgs)
{
    std::ostringstream ss;
    ss << pgs.fixedOjama
       << "(" << pgs.pendingOjama << ")";
    printText(locate(playerId, 0, 0), ss.str());
}

void Cui::printPuyo(const Location& location, const ColoredText& text)
{
    auto& prev = printPuyoCache_[location];
    if (prev == text)
        return;
    setCursor(location);
    setColor(text.color);
    cout << text.text;
    prev = text;
}

void Cui::printText(const Location& location, const string& text)
{
    auto& prev = printTextCache_[location];
    if (prev == text)
        return;

    setCursor(location);
    cout << text;
    if (text.size() < prev.size())
        cout << string(prev.size() - text.size(), ' ');
    printTextCache_[location] = text;
}

void Cui::setCursor(const Location& location)
{
    setCursor(location.x, location.y);
}

void Cui::setCursor(int x, int y)
{
    cout << "\x1b[" << y << ";" << x << "H";
}

void Cui::setColor(PuyoColor color)
{
    string colorCode(C_BLACK);
    switch (color) {
    case PuyoColor::RED:
        colorCode = C_RED;
        break;
    case PuyoColor::BLUE:
        colorCode = C_BLUE;
        break;
    case PuyoColor::GREEN:
        colorCode = C_GREEN;
        break;
    case PuyoColor::YELLOW:
        colorCode = C_YELLOW;
        break;
    default:
        break;
    }
    cout << colorCode;
}

Cui::Location Cui::locate(int x, int y)
{
    return Location(x, y);
}

Cui::Location Cui::locate(int playerId, int x, int y)
{
    int posX = 1 + 30 * playerId;
    int posY = 1;
    return Location(posX + x, posY + y);
}

Cui::ColoredText Cui::puyoText(PuyoColor color, int y)
{
    string text;
    if (color == PuyoColor::OJAMA) {
        text = "@@";
    } else if (color == PuyoColor::WALL) {
        text = "##";
    } else {
        if (y == 13)
            text = "__";
        else
            text = "  ";
    }

    return ColoredText(color, text);
}
