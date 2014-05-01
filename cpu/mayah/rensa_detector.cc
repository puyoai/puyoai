#include "rensa_detector.h"

#include <iostream>
#include <set>
#include <vector>

#include "field.h"
#include "plan.h"
#include "rensa_info.h"

using namespace std;

// TODO(mayah): 到達できないところから発火できてしまう

// TODO(mayah): findPossibleRensas が現状遅すぎ、使いものにならない。
// numExtraAddedPuyos が 3 で、現状の 1 と同程度の速度を出せるようにする必要がある。

// TODO(mayah): numExtraAddedPuyos ごとに連鎖情報を得るようにしたい

struct NecessaryRensaPuyo {
    NecessaryRensaPuyo(int x, PuyoColor c, int n) :
        x(x), c(c), n(n) {}

    friend bool operator<(const NecessaryRensaPuyo& lhs, const NecessaryRensaPuyo& rhs) {
        if (lhs.x != rhs.x)
            return lhs.x < rhs.x;
        if (lhs.c != rhs.c)
            return static_cast<int>(lhs.c) < static_cast<int>(rhs.c);

        return lhs.n < rhs.n;
    }

    static PuyoSet toPuyoSet(const vector<NecessaryRensaPuyo>& necessaryRensaPuyos) {
        PuyoSet puyoSet;
        for (auto it = necessaryRensaPuyos.begin(); it != necessaryRensaPuyos.end(); ++it)
            puyoSet.add(it->c, it->n);

        return puyoSet;
    }

    int x;
    PuyoColor c;
    int n;
};

static inline void simulateInternal(Field& f, PossibleRensaInfo& info, int additionalChains)
{
    info.rensaInfo = f.simulate(1 + additionalChains);
}

static inline void simulateInternal(Field& f, TrackedPossibleRensaInfo& info, int additionalChains)
{
    info.rensaInfo = f.simulateAndTrack(&info.trackResult, 1 + additionalChains);
}

template<typename T, typename AfterSimulationCallback>
static void findRensasInternal(const Field& field, int additionalChains, AfterSimulationCallback callback)
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = field.height(x); y >= 1; --y) {
            PuyoColor c = field.color(x, y);

            DCHECK(c != EMPTY);
            if (c == OJAMA)
                continue;

            // Drop puyo on
            for (int d = -1; d <= 1; ++d) {
                if (x + d <= 0 || Field::WIDTH < x + d)
                    continue;
                if (d == 0) {
                    if (field.color(x, y + 1) != EMPTY)
                        continue;
                } else {
                    if (field.color(x + d, y) != EMPTY)
                        continue;
                }

                Field f(field);
                int necessaryPuyos = 0;
                while (necessaryPuyos <= 4 && f.connectedPuyoNums(x, y) < 4 && f.height(x + d) <= 13) {
                    f.dropPuyoOn(x + d, c);
                    ++necessaryPuyos;
                }

                if (necessaryPuyos > 4)
                    continue;

                NecessaryRensaPuyo necessaryRensaPuyo(x + d, c, necessaryPuyos);

                T info;
                simulateInternal(f, info, additionalChains);

                callback(f, info, necessaryRensaPuyo);
            }            
        }
    }
}

template<typename T>
static void findPossibleRensasInternal(std::vector<T>& result, const Field& field, PuyoSet addedSet, int leftX, int restAdded)
{
    findRensasInternal<T>(field, 0, [&result, addedSet](const Field&, T& info, const NecessaryRensaPuyo& necessaryRensaPuyo) {
            info.necessaryPuyoSet.add(addedSet);
            info.necessaryPuyoSet.add(necessaryRensaPuyo.c, necessaryRensaPuyo.n);
            result.push_back(info);
    });

    if (restAdded <= 0)
        return;

    Field f(field);
    for (int x = leftX; x <= Field::WIDTH; ++x) {
        if (f.height(x) >= 13)
            continue;

        for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
            PuyoColor c = normalPuyoColorOf(i);

            f.dropPuyoOn(x, c);
            addedSet.add(c, 1);

            if (f.connectedPuyoNums(x, f.height(x)) < 4)
                findPossibleRensasInternal(result, f, addedSet, x, restAdded - 1);

            f.removeTopPuyoFrom(x);
            addedSet.add(c, -1);
        }
    }
}

void RensaDetector::findPossibleRensas(std::vector<PossibleRensaInfo>& result,
                                       const Field& field,                                       
                                       int numExtraAddedPuyos,
                                       const PuyoSet& additionalPuyoSet)
{
    findPossibleRensasInternal(result, field, additionalPuyoSet, 1, numExtraAddedPuyos);
}

void RensaDetector::findPossibleRensas(std::vector<TrackedPossibleRensaInfo>& result,
                                       const Field& field,
                                       int numExtraAddedPuyos,
                                       const PuyoSet& additionalPuyoSet)
{
    findPossibleRensasInternal(result, field, additionalPuyoSet, 1, numExtraAddedPuyos);
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

void RensaDetector::findPossibleRensasUsingIteration(vector<vector<TrackedPossibleRensaInfo>>& result, const Field& field, int numKeyPuyos)
{
    if (numKeyPuyos < 0)
        return;

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

void RensaDetector::findFeasibleRensas(vector<FeasibleRensaInfo>& result, const Field& field, int numKumiPuyo, const vector<KumiPuyo>& kumiPuyos)
{
    vector<Plan> plans;
    plans.reserve(22 + 22*22 + 22*22*22);
    findAvailablePlans(field, numKumiPuyo, kumiPuyos, plans);

    for (vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        if (!it->isRensaPlan())
            continue;

        result.push_back(FeasibleRensaInfo(
                             BasicRensaResult(it->totalChains(),
                                              it->totalScore(),
                                              it->totalFrames() - it->initiatingFrames()),
                             it->initiatingFrames()));
    }
}
