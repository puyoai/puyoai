#include "puyofu.h"

#include <glog/logging.h>

#include <core/field.h>
#include <core/state.h>
#include <util/field_util.h>

void PuyoFu::setField(int pi, const Field& f, int state, int time) {
  state &= ~STATE_YOU_CAN_PLAY;
  if (!state)
    return;

  if ((state & STATE_YOU_GROUNDED) == 0)
    return;

  Move* move = new Move();
  move->pi = pi;
  move->f.reset(new Field(f));
  move->next[6] = 0;
  for (int i = 0; i < 6; i++) {
    move->next[i] = f.GetNextPuyo(i) + '0';
  }
  move->time = time;
  moves_.push_back(move);
}

void PuyoFu::emitFieldTransitionLog(FILE* fp, int pi) const {
  Field f;
  for (size_t i = 0; i < moves_.size(); i++) {
    Move* m = moves_[i];
    if (m->pi != pi)
      continue;

    string before, after;
    GetRensimQueryString(f, &before);
    GetRensimQueryString(*m->f, &after);
    if (before.empty()) {
      if (after.empty())
        continue;
      before.push_back('0');
    }
    if (after.empty())
      after.push_back('0');
    fprintf(fp, "%s %s %s\n", before.c_str(), m->next, after.c_str());
    f = *m->f;
  }
}

