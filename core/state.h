#ifndef __STATE_H__
#define __STATE_H__

enum State {
  STATE_NONE = 0,
  STATE_YOU_CAN_PLAY = 1 << 0,
  STATE_WNEXT_APPEARED = 1 << 2,
  STATE_YOU_GROUNDED = 1 << 4,
  /* STATE_YOU_WIN = 1 << 6, */
  STATE_CHAIN_DONE = 1 << 8,
  STATE_OJAMA_DROPPED = 1 << 10,
};

#endif  // __STATE_H__
