#include "core/bit_field.h"

#include <smmintrin.h>

#include "core/score.h"

// static
const FieldBits BitField::s_empty_;

int BitField::vanish(int chain, FieldBits* erased)
{
    int numErased = 0;
    int numColors = 0;
    int longBonusCoef = 0;

    *erased = FieldBits();

    for (PuyoColor c : NORMAL_PUYO_COLORS) {
        FieldBits mask = bits(c).masked();
        FieldBits seed = mask.vanishingSeed();

        if (seed.isEmpty())
            continue;

        ++numColors;
        seed.iterateBit([&](FieldBits x) {
            if (mask.testz(x))
                return;

            FieldBits expanded = x.expand4(mask).expand(mask);
            int count = expanded.popcount();
            numErased += count;
            longBonusCoef += longBonus(count);
            erased->setAll(expanded);
            mask.unsetAll(expanded);
        });
    }

    // Removes ojama.
    FieldBits ojama(erased->expand1(bits(PuyoColor::OJAMA)));

    erased->setAll(ojama);
    for (auto& c : colors_)
        c.unsetAll(*erased);

    return 10 * numErased * calculateRensaBonusCoef(chainBonus(chain) , longBonusCoef, colorBonus(numColors));
}
