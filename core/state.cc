#include "state.h"

using namespace std;

string GetStateString(int st) {
  string r;
  r.resize(5);
  r[0] = ((st & STATE_YOU_CAN_PLAY) != 0) ? 'P' : '-';
  r[1] = ((st & STATE_WNEXT_APPEARED) != 0) ? 'W' : '-';
  r[2] = ((st & STATE_YOU_GROUNDED) != 0) ? 'G' : '-';
  r[3] = ((st & STATE_CHAIN_DONE) != 0) ? 'C' : '-';
  r[4] = ((st & STATE_OJAMA_DROPPED) != 0) ? 'O' : '-';
  return r;
}
