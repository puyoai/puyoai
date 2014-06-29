#ifndef CORE_ALGORITHM_RENSA_DETECTOR_INL_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_INL_H_

template<typename SimulationCallback>
void RensaDetector::findRensas(const CoreField& field, Mode mode, SimulationCallback callback)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = field.height(x); y >= 1; --y) {
          switch (mode) {
            case DROP:
              tryDropFire(x, y, field, callback);
            case FLOAT:
              tryFloatFire(x, y, field, callback);
          }
        }
    }
}

template<typename SimulationCallback>
static inline void tryDropFire(
    int x, int y, const CoreField& field, const SimulationCallback& callback) {
    PuyoColor c = field.color(x, y);

    DCHECK(c != PuyoColor::EMPTY);
    if (c == PuyoColor::OJAMA)
        return;

    // Drop puyo on
    for (int d = -1; d <= 1; ++d) {
        if (x + d <= 0 || CoreField::WIDTH < x + d)
            continue;
        if (d == 0) {
            if (field.color(x, y + 1) != PuyoColor::EMPTY)
                continue;
        } else {
            if (field.color(x + d, y) != PuyoColor::EMPTY)
                continue;
        }

        CoreField f(field);
        int necessaryPuyos = 0;
        while (necessaryPuyos <= 4 && f.countConnectedPuyos(x, y) < 4 && f.height(x + d) <= 13) {
            f.dropPuyoOn(x + d, c);
            ++necessaryPuyos;
        }

        if (necessaryPuyos > 4)
            continue;

        callback(&f, x + d, c, necessaryPuyos);
    }

}

template<typename SimulationCallback>
static inline void tryFloatFire(
    int x, int y, const CoreField& field, const SimulationCallback& callback) {
  PuyoColor c = field.color(x, y);

  DCHECK(c != PuyoColor::EMPTY);
  if (c == PuyoColor::OJAMA)
    return;

  int necessaryPuyos = 4 - field.countConnectedPuyos(x, y);
  CoreField f(field);

  int dx = x - 1;
  // float puyo col dx
  for (; dx <= x + 1 && necessaryPuyos > 0; ++dx) {
    if (dx <= 0 || CoreField::WIDTH < dx) {
     continue;
    }


    // Check y
    if (dx != x) {
      if (field.color(dx, y) != PuyoColor::EMPTY) {
        continue;
      } else { // necessaryPuyos must be more than 0
        f.unsafeSet(dx, y, c);
        --necessaryPuyos;
      }
    }

    int dy_min = y - 1;
    // Check under y
    for (; necessaryPuyos > 0 && dy_min > 0 && field.color(dx ,dy_min) == PuyoColor::EMPTY;
        --dy_min) {
      f.unsafeSet(dx, dy_min, c);
      --necessaryPuyos;
    }

    // Check over y
    for (int dy = y + 1;
        necessaryPuyos > 0 && dy <= 13 && field.color(dx ,dy) == PuyoColor::EMPTY; ++dy) {
      f.unsafeSet(dx, dy, c);
      --necessaryPuyos;
    }

    // Fill ojama
    for(; dy_min > 0 && field.color(dx, dy_min) == PuyoColor::EMPTY; --dy_min) {
      f.unsafeSet(dx, dy_min, PuyoColor::OJAMA);
    }

    f.recalcHeightOn(dx);
  }

  if (necessaryPuyos <= 0) {
    callback(&f, dx, c, necessaryPuyos);
  }
}

#endif

