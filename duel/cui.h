#ifndef DUEL_CUI_H_
#define DUEL_CUI_H_

#include <iostream>
#include <string>

class FieldRealtime;

class Cui {
 public:
  static void Clear();
  static void Print(int player_id, const FieldRealtime& field, const std::string& debug_message);
  static void PrintField(int player_id, const FieldRealtime& field);
  static void PrintNextPuyo(int player_id, const FieldRealtime& field);
  static void PrintOjamaPuyo(int player_id, const FieldRealtime& field);
  static void PrintDebugMessage(int player_id, const std::string& debug_message);
};

#endif  // DUEL_CUI_H
