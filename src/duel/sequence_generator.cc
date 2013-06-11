#include "duel/sequence_generator.h"

#include <algorithm>
#include <random>
#include <vector>

using namespace std;

KumipuyoSeq generateSequence()
{
    // AC puyo2 algorithm
    // make 64 x 4 sequece -> shuffle
    // if the first 3 hands contains 4 colors, make them 3 colors.
    //
    // In our algorithm, make it easy a little bit.
    // first, we shuffle [0, 64*3), then, shuffle [6, 64*4).
    vector<PuyoColor> colors(256);
    for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
        PuyoColor c = normalPuyoColorOf(i);
        for (int j = 0; j < 64; ++j) {
            colors[i * 64 + j] = c;
        }
    }

    // TODO(mayah): Maybe srand() is used somewhere? We need to deprecate it.
    random_device rd;
    mt19937 mt(rd());
    shuffle(colors.begin(), colors.begin() + 64 * 3, mt);
    shuffle(colors.begin() + 6, colors.end(), mt);

#if 0
    const char* puyo_seq = getenv("PUYO_SEQ");
    if (puyo_seq) {
        for (int i = 0; i < 256 && puyo_seq[i]; i++) {
            char c = puyo_seq[i];
            if (c < '4' || c > '7') {
                cerr << "Broken PUYO_SEQ=" << puyo_seq << endl;
                abort();
            }
            sequence[i] = puyo_seq[i];
        }
    }
#endif

    vector<Kumipuyo> seq(128);
    for (int i = 0; i < 128; ++i) {
        seq[i].axis = colors[2 * i];
        seq[i].child = colors[2 * i + 1];
    }

    return KumipuyoSeq(seq);
}
