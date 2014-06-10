#include "duel/field_realtime.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "duel/cui.h"
#include "duel/game_state.h"

using namespace std;

string Locate(int player_id, int x, int y) {
  int pos_x = 1 + 30 * player_id;
  int pos_y = 1;

  stringstream ss;
  ss << "\x1b[" << (pos_y + y) << ";" << (pos_x + x) << "H";
  return ss.str();
}

string Locate(int x, int y) {
  stringstream ss;
  ss << "\x1b[" << y << ";" << x << "H";
  return ss.str();
}

string GetPuyoText(char color, int y = 0) {
  const char C_RED[] = "\x1b[41m";
  const char C_BLUE[] = "\x1b[44m";
  const char C_GREEN[] = "\x1b[42m";
  const char C_YELLOW[] = "\x1b[43m";
  const char C_BLACK[] = "\x1b[49m";

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
    case RED: color_code = C_RED; break;
    case BLUE: color_code = C_BLUE; break;
    case GREEN: color_code = C_GREEN; break;
    case YELLOW: color_code = C_YELLOW; break;
    default: color_code = C_BLACK; break;
  }

  return color_code + text + C_BLACK;
}

void Cui::clear()
{
    cout << "\x1b[2J";
}

void Cui::onUpdate(const GameState& gameState)
{
    Print(0, gameState.field(0), gameState.message(0));
    Print(1, gameState.field(1), gameState.message(1));
}

void Cui::PrintField(int player_id, const FieldRealtime& field)
{
  int x1, y1, x2, y2, r;
  PuyoColor c1, c2;
  field.GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);

  for (int y = 0; y < CoreField::MAP_HEIGHT; y++) {
    for (int x = 0; x < CoreField::MAP_WIDTH; x++) {
      PuyoColor color = field.field().color(x, y);
      if (field.IsInUserState()) {
        if (x == x1 && y == y1) {
          color = c1;
        }
        if (x == x2 && y == y2) {
          color = c2;
        }
      }

      cout << Locate(player_id, x * 2, CoreField::MAP_HEIGHT - y);
      cout << GetPuyoText(color, y);
    }
  }
}

void Cui::PrintNextPuyo(int player_id, const FieldRealtime& field)
{
  // Next puyo info
  for (int i = 2; i < 6; i++) {
    const string location =
      Locate(player_id, 9 * 2, 3 + (i - 2) + ((i - 2) / 2));
    cout << location << GetPuyoText(field.GetNextPuyo(i));
  }
}

void Cui::PrintDebugMessage(int player_id, const string& debug_message)
{
  // "\x1B[0K" clears the line
  if (!debug_message.empty()) {
    cout << Locate(1, 1 + CoreField::MAP_HEIGHT + 3 + player_id)
	 << "\x1B[0K" << debug_message << flush;
  }
  cout << Locate(1, 1 + CoreField::MAP_HEIGHT + 5) << flush;
}

void Cui::Print(int player_id, const FieldRealtime& field, const string& debug_message)
{
    PrintField(player_id, field);
    PrintOjamaPuyo(player_id, field);
    PrintNextPuyo(player_id, field);
    PrintDebugMessage(player_id, debug_message);

    // Score
    cout << Locate(player_id, 0, CoreField::MAP_HEIGHT + 1) << setw(10) << field.score();
}

void Cui::PrintOjamaPuyo(int player_id, const FieldRealtime& field)
{
    cout << Locate(player_id, 0, 0) << field.numFixedOjama()
         << "(" << field.numPendingOjama() << ")          ";
}
