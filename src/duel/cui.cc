#include "duel/field_realtime.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "duel/cui.h"
#include "duel/game_state.h"

using std::string;
using std::cout;

namespace {

const char C_RED[] = "\x1b[41m";
const char C_BLUE[] = "\x1b[44m";
const char C_GREEN[] = "\x1b[42m";
const char C_YELLOW[] = "\x1b[43m";
const char C_BLACK[] = "\x1b[49m";

string Locate(int x, int y)
{
    std::stringstream ss;
    ss << "\x1b[" << y << ";" << x << "H";
    return ss.str();
}

string Locate(int player_id, int x, int y)
{
    int pos_x = 1 + 30 * player_id;
    int pos_y = 1;
    return Locate(pos_x + x, pos_y + y);
}

string GetPuyoText(PuyoColor color, int y = 0)
{
    string text;
    if (color == OJAMA) {
        text = "@@";
    } else if (color == WALL) {
        text = "##";
    } else {
        if (y == 13)
            text = "__";
        else
            text = "  ";
    }

    string color_code;
    switch (color) {
    case PuyoColor::RED:
        color_code = C_RED;
        break;
    case PuyoColor::BLUE:
        color_code = C_BLUE;
        break;
    case PuyoColor::GREEN:
        color_code = C_GREEN;
        break;
    case PuyoColor::YELLOW:
        color_code = C_YELLOW;
        break;
    default:
        color_code = C_BLACK;
        break;
    }

    return color_code + text + C_BLACK;
}

} // namespace

void Cui::clear()
{
    cout << "\x1b[2J" << std::flush;
    print_puyo_cache_.clear();
    print_text_cache_.clear();
}

void Cui::newGameWillStart()
{
    cout << Locate(1, 1 + CoreField::MAP_HEIGHT + 3)
         << "\x1B[0K"
         << Locate(1, 1 + CoreField::MAP_HEIGHT + 4)
         << "\x1B[0K" << std::flush;
    print_puyo_cache_.clear();
    print_text_cache_.clear();
}

void Cui::onUpdate(const GameState& gameState)
{
    Print(0, gameState.field(0), gameState.message(0));
    Print(1, gameState.field(1), gameState.message(1));
    std::cout << std::flush;
}

void Cui::PrintField(int player_id, const FieldRealtime& field)
{
    Kumipuyo kumipuyo = field.kumipuyo();
    KumipuyoPos kumipuyoPos = field.kumipuyoPos();

    for (int y = 0; y < CoreField::MAP_HEIGHT; y++) {
        for (int x = 0; x < CoreField::MAP_WIDTH; x++) {
            PuyoColor color = field.field().color(x, y);
            if (field.userPlayable()) {
                if (x == kumipuyoPos.axisX() && y == kumipuyoPos.axisY())
                    color = kumipuyo.axis;
                if (x == kumipuyoPos.childX() && y == kumipuyoPos.childY())
                    color = kumipuyo.child;
            }

            PrintPuyo(Locate(player_id, x * 2, CoreField::MAP_HEIGHT - y),
                      GetPuyoText(color, y));
        }
    }
}

void Cui::PrintNextPuyo(int player_id, const FieldRealtime& field)
{
    static const NextPuyoPosition npp[] = {
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
    };

    // Next puyo info
    for (int i = 0; i < 4; ++i) {
        PrintPuyo(Locate(player_id, 9 * 2, 3 + i + (i / 2)),
                  GetPuyoText(field.puyoColor(npp[i])));
    }
}

void Cui::PrintDebugMessage(int player_id, const string& debug_message)
{
    if (!debug_message.empty()) {
        PrintText(Locate(1, 1 + CoreField::MAP_HEIGHT + 3 + player_id),
                  debug_message);
    }
}

void Cui::Print(int player_id, const FieldRealtime& field,
                const string& debug_message)
{
    PrintField(player_id, field);
    PrintOjamaPuyo(player_id, field);
    PrintNextPuyo(player_id, field);
    PrintDebugMessage(player_id, debug_message);
    PrintScore(player_id, field.score());

    // Set cursor
    cout << Locate(0, CoreField::MAP_HEIGHT + 3);
}

void Cui::PrintScore(int player_id, int score)
{
    std::ostringstream ss;
    ss << std::setw(10) << score;
    PrintText(Locate(player_id, 0, CoreField::MAP_HEIGHT + 1),
              ss.str());
}

void Cui::PrintOjamaPuyo(int player_id, const FieldRealtime& field)
{
    std::ostringstream ss;
    ss << field.numFixedOjama()
       << "(" << field.numPendingOjama() << ")";
    PrintText(Locate(player_id, 0, 0), ss.str());
}

void Cui::PrintPuyo(const string& location, const string& text)
{
    auto& prev = print_puyo_cache_[location];
    if (prev == text)
        return;
    cout << location << text;
    prev = text;
}

void Cui::PrintText(const string& location, const string& text)
{
    auto& prev = print_text_cache_[location];
    if (prev == text)
        return;

    cout << location << text;
    for (size_t i = prev.size(); i < text.size(); ++i)
        cout << ' ';
    prev = text;
}
