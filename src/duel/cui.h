#ifndef DUEL_CUI_H_
#define DUEL_CUI_H_

#include <iostream>
#include <string>
#include <unordered_map>

#include "base/base.h"
#include "core/puyo_color.h"
#include "core/server/game_state_observer.h"

class GameState;
struct PlayerGameState;

class Cui : public GameStateObserver {
public:
    Cui() {}
    virtual ~Cui() {}

    virtual void clear();
    void flush();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;

protected:
    // Figures where the cursor is. {x=1, y=1} figures the top-left corner.
    struct Location {
        int x, y;
        Location(int a, int b) : x(a), y(b) {}
        bool operator==(const Location& rhs) const { return x == rhs.x && y == rhs.y; }
        bool operator!=(const Location& rhs) const { return !(this->operator==(rhs)); }
        struct Hash {
            std::size_t operator()(const Location& key) const {
                return std::hash<int>()(key.x + key.y * 1000);
            }
        };
    };
    // TODO(peria): Remove this struct. We don't have to keep both color and text.
    struct ColoredText {
        PuyoColor color;
        std::string text;
        ColoredText() : color(PuyoColor::EMPTY), text("") {}
        ColoredText(PuyoColor c, const std::string& t) : color(c), text(t) {}
        ColoredText(const std::string& t) : color(PuyoColor::EMPTY), text(t) {}
        bool operator==(const ColoredText& rhs) const { return color == rhs.color && text == rhs.text; }
        bool operator!=(const ColoredText& rhs) const { return !(this->operator==(rhs)); }
    };

    void print(int playerId, const PlayerGameState&);
    void printField(int playerId, const PlayerGameState&);
    void printNextPuyo(int playerId, const PlayerGameState&);
    void printScore(int playerId, int score);
    void printOjamaPuyo(int playerId, const PlayerGameState&);
    void printMessage(int playerId, const std::string& message);

    void printPuyo(const Location& location, const ColoredText& text);
    void printText(const Location& location, const std::string& text);

    virtual void setCursor(const Location& location);
    virtual void setCursor(int x, int y);
    virtual void setColor(PuyoColor c);
    static Location locate(int x, int y);
    static Location locate(int playerId, int x, int y);
    static ColoredText puyoText(PuyoColor color, int y = 0);

    std::unordered_map<Location, ColoredText, Location::Hash> printPuyoCache_;
    std::unordered_map<Location, std::string, Location::Hash> printTextCache_;
};

#endif  // DUEL_CUI_H_
