#include "cpu/mayah/rensa_detector.h"

#include <vector>

#include "cpu/mayah/field.h"

#if 0
using namespace std;

// TODO(mayah): copied from core_algorithm.
// Needs to be merged.


static inline void simulateInternal(Field& f, PossibleRensaInfo& info, int additionalChains)
{
    info.rensaInfo = f.simulate(1 + additionalChains);
}

static inline void simulateInternal(Field& f, TrackedPossibleRensaInfo& info, int additionalChains)
{
    info.rensaInfo = f.simulateAndTrack(&info.trackResult, 1 + additionalChains);
}

static void findPossibleRensasUsingIterationInternal(vector<vector<NecessaryRensaPuyo>>& results,
                                                     const Field& field,
                                                     const vector<NecessaryRensaPuyo>& alreadyAddedPuyos,
                                                     int restIteration)
{
    if (restIteration <= 0)
        return;

    findRensasInternal<PossibleRensaInfo>(field, 0, [&results, &alreadyAddedPuyos, restIteration](const Field& f, PossibleRensaInfo& /*info*/, const NecessaryRensaPuyo& necessaryRensaPuyo) {
            vector<NecessaryRensaPuyo> newNecessaryRensaPuyos(alreadyAddedPuyos);
            newNecessaryRensaPuyos.push_back(necessaryRensaPuyo);
            results.push_back(newNecessaryRensaPuyos);

            findPossibleRensasUsingIterationInternal(results, f, newNecessaryRensaPuyos, restIteration - 1);
        });
}

vector<vector<TrackedPossibleRensaInfo>>
RensaDetectorEx::findPossibleRensasUsingIteration(const Field& field, int numKeyPuyos)
{
    DCHECK(0 <= numKeyPuyos);

    vector<vector<NecessaryRensaPuyo>> necessaryRensaPuyoss;
    findPossibleRensasUsingIterationInternal(necessaryRensaPuyoss, field, vector<NecessaryRensaPuyo>(), numKeyPuyos + 1);

    result.resize(numKeyPuyos + 1);
    for (const vector<NecessaryRensaPuyo>& necessaryRensaPuyos : necessaryRensaPuyoss) {
        DCHECK(!necessaryRensaPuyos.empty());
        DCHECK(static_cast<int>(necessaryRensaPuyos.size()) <= numKeyPuyos + 1) << ' ' << necessaryRensaPuyos.size() << ' ' << numKeyPuyos;

        // Find initial puyo
        int initialX = -1;
        int initialY = -1;
        PuyoColor initialColor = necessaryRensaPuyos[0].c;

        Field f(field);

        // 必要なぷよを落としてみる。ここで、消えてしまったり、おけなかったりしたら、これ以上考慮しなくて良い。
        // ここで、j == 0 のときは、必要数 - 1 だけ落とす。
        PuyoSet puyoSet;
        bool ok = true;
        for (size_t j = 0; j < necessaryRensaPuyos.size(); ++j) {
            int x = necessaryRensaPuyos[j].x;
            PuyoColor c = necessaryRensaPuyos[j].c;
            int n = necessaryRensaPuyos[j].n;

            if (j == 0) {
                if (n > 0)
                    n -= 1;

                if (f.color(x, field.height(x)) == c) {
                    initialX = x;
                    initialY = field.height(x);
                } else if (f.color(x - 1, field.height(x) + 1) == c) {
                    initialX = x - 1;
                    initialY = field.height(x) + 1;
                } else if (f.color(x + 1, field.height(x) + 1) == c) {
                    initialX = x + 1;
                    initialY = field.height(x) + 1;
                } else {
                    DCHECK(n > 0) << ' ' << n << ' ' << c << ' ' << x << ' ' << f.debugOutput();
                    initialX = x;
                    initialY = field.height(x) + 1;
                }
            }

            if (f.height(x) + n >= 14) {
                ok = false;
                break;
            }

            // TODO(mayah): n can be 3 at most, so we would like to do loop inlining?
            for (int k = 0; k < n; ++k) {
                f.dropPuyoOn(x, c);
                puyoSet.add(c, 1);
            }

            if (n > 0 && f.connectedPuyoNums(x, f.height(x)) >= 4) {
                ok = false;
                break;
            }
        }

        if (!ok)
            continue;

        // TODO(mayah): 空気に触れているけど空中に浮いているのはどうするべきか？
        // 上乗せすれば打てるはずだが、とりあえずは考えないことにする。
        int breathingX, breathingY;
        if (f.findBestBreathingSpace(breathingX, breathingY, initialX, initialY)) {
            if (f.color(breathingX, breathingY - 1) == EMPTY)
                continue;

            f.dropPuyoOn(breathingX, initialColor);

            TrackedPossibleRensaInfo info;
            info.necessaryPuyoSet.add(puyoSet);
            info.necessaryPuyoSet.add(initialColor, 1);
            info.rensaInfo = f.simulateAndTrack(&info.trackResult);

            DCHECK(necessaryRensaPuyos.size() >= 1);
            result[necessaryRensaPuyos.size() - 1].push_back(info);
        }
    }
}
#endif

