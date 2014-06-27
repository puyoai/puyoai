#ifndef CORE_ALGORITHM_RENSA_DETECTOR_INL_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_INL_H_

template<typename SimulationCallback>
void RensaDetector::findRensas(const CoreField& field, SimulationCallback callback)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = field.height(x); y >= 1; --y) {
            PuyoColor c = field.color(x, y);

            DCHECK(c != PuyoColor::EMPTY);
            if (c == PuyoColor::OJAMA)
                continue;

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
    }
}

#endif

