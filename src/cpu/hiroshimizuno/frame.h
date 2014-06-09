#ifndef __CPU_HIROSHIMIZUNO_FRAME_H__
#define __CPU_HIROSHIMIZUNO_FRAME_H__

#include <cstdlib>
#include <sstream>
#include <string>

struct __attribute((aligned (16))) Frame {
  unsigned char __attribute__((aligned (16))) field[96];
  unsigned char __attribute__((aligned (16))) field_o[96];
  unsigned long long nexts;
  unsigned long long nexts_o;
  int current_x;
  int current_y;
  int current_r;
  int ojama;
  int current_x_o;
  int current_y_o;
  int current_r_o;
  int ojama_o;
  int score;
  int score_o;
  int ack;
  int nack;
  int id;
  int win;
  int lose;
  bool state_none;
  bool state_canmove;
  bool state_canmove_o;
  bool state_gotnexts;
  bool state_gotnexts_o;
  bool state_settled;
  bool state_settled_o;
  bool state_won;
  bool state_won_o;
  bool state_chained;
  bool state_chained_o;
};

void ParseField(const std::string& seventwo, unsigned char field[96]) {
  unsigned char buf[96] = {0};
  int offset = 0;
  if (seventwo.size() == 78) {
    for (offset = 0; offset < 6; ++offset) {
      unsigned char c = seventwo[offset];
      c -= 0x30;
      if (c >= 4) { c = (c - 3) * 2; }
      buf[offset + 90] = c;
    }
  }
  for (int pos = 0; pos < seventwo.size(); ++pos) {
    unsigned char c = seventwo[offset + pos];
    c -= 0x30;
    if (c >= 4) { c = (c - 3) * 2; }
    buf[pos] = c;
  }
  for (int x = 0; x < 6; ++x) {
    for (int y = 0; y < 16; ++y) {
      field[x * 6 + 16] = buf[(15 - y) * 6 + x];
    }
  }
}

void ParseNexts(const std::string& six, unsigned long long* nexts) {
  *nexts = 0;
  for (int i = 0; i < 6; ++i) {
    unsigned char c = six[i];
    c -= (0x30 + 3);
    c *= 2;
    ((unsigned char*) nexts)[i] = c;
  }
}

void ParseState(const std::string& number, Frame* frame) {
  int enumed = atoi(number.c_str());
  frame->state_none = (enumed == 0);
  frame->state_canmove = (enumed & 1);
  frame->state_canmove_o = (enumed & 2);
  frame->state_gotnexts = (enumed & 4);
  frame->state_gotnexts_o = (enumed & 8);
  frame->state_settled = (enumed & 16);
  frame->state_settled_o = (enumed & 32);
  frame->state_won = (enumed & 64);
  frame->state_won_o = (enumed & 128);
  frame->state_chained = (enumed & 256);
  frame->state_chained_o = (enumed & 512);
}

void NewGame(Frame* frame) {
  frame->ack = 0;
  frame->nack = 0;
}

void GameEnd(int win, Frame* frame) {
  if (win > 0) { frame->win += 1; }
  else if (win < 0) { frame->lose += 1; }
}

bool ParseLine(const std::string& line, Frame* frame) {
  std::istringstream input(line);
  int num_chunks = 0, num_chunks_invalid = 0;
  for (std::string chunk; input >> chunk; ++num_chunks) {
    const std::string three = chunk.substr(0, 3);
    if (three == "ID=") { frame->id = atoi(chunk.substr(3).c_str()); if (frame->id < 2) { NewGame(frame); } }
    else if (three == "YF=") { ParseField(chunk.substr(3), frame->field); }
    else if (three == "OF=") { ParseField(chunk.substr(3), frame->field_o); }
    else if (three == "YP=") { ParseNexts(chunk.substr(3), &(frame->nexts)); }
    else if (three == "OP=") { ParseNexts(chunk.substr(3), &(frame->nexts_o)); }
    else if (three == "YX=") { frame->current_x = atoi(chunk.substr(3).c_str()); }
    else if (three == "YY=") { frame->current_y = atoi(chunk.substr(3).c_str()); }
    else if (three == "YR=") { frame->current_r = atoi(chunk.substr(3).c_str()); }
    else if (three == "OX=") { frame->current_x_o = atoi(chunk.substr(3).c_str()); }
    else if (three == "OY=") { frame->current_y_o = atoi(chunk.substr(3).c_str()); }
    else if (three == "OR=") { frame->current_r_o = atoi(chunk.substr(3).c_str()); }
    else if (three == "YO=") { frame->ojama = atoi(chunk.substr(3).c_str()); }
    else if (three == "OO=") { frame->ojama_o = atoi(chunk.substr(3).c_str()); }
    else if (three == "YS=") { frame->score = atoi(chunk.substr(3).c_str()); }
    else if (three == "OS=") { frame->score_o = atoi(chunk.substr(3).c_str()); }
    else if (chunk.substr(0, 4) == "END=") { GameEnd(atoi(chunk.substr(4).c_str()), frame); }
    else if (chunk.substr(0, 4) == "ACK=") { frame->ack = atoi(chunk.substr(4).c_str()); }
    // TODO(hiroshimizuno): NACK may contain several frame IDs, separated by commas.
    else if (chunk.substr(0, 5) == "NACK=") { frame->nack = atoi(chunk.substr(5).c_str()); }
    else if (chunk.substr(0, 6) == "STATE=") { ParseState(chunk.substr(6), frame); }
    else { num_chunks_invalid += 1; }
  }
  return (num_chunks >= 16 && num_chunks_invalid == 0);
}

#endif  // __CPU_HIROSHIMIZUNO_FRAME_H__
