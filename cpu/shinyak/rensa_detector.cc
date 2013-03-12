#include "rensa_detector.h"

#include "field.h"
#include "plan.h"
#include "rensa_info.h"

using namespace std;

// TODO(mayah): findPossibleRensas が現状遅すぎ、使いものにならない。
// numExtraAddedPuyos が 3 程度で、現状の 1 と同程度の速度を出せるようにする必要がある。
// 枝刈り？

struct NecessaryRensaPuyo {
    NecessaryRensaPuyo(int x, PuyoColor c, int numNecessaryPuyos) :
        x(x), c(c), numNecessaryPuyos(numNecessaryPuyos) {}

    static PuyoSet toPuyoSet(const vector<NecessaryRensaPuyo>& necessaryRensaPuyos) {
        PuyoSet puyoSet;
        for (auto it = necessaryRensaPuyos.begin(); it != necessaryRensaPuyos.end(); ++it)
            puyoSet.add(it->c, it->numNecessaryPuyos);

        return puyoSet;
    }

    const int x;
    const PuyoColor c;
    const int numNecessaryPuyos;
};

static inline void simulateInternal(Field& f, PossibleRensaInfo& info, int additionalChains)
{
    f.simulate(info.rensaInfo, additionalChains);
}

static inline void simulateInternal(Field& f, TrackedPossibleRensaInfo& info, int additionalChains)
{
    f.simulateAndTrack(info.rensaInfo, info.trackResult, additionalChains);
}

template<typename AfterSimulationCallback, typename T>
static void findRensasInternal(vector<T>& result, const Field& field, int additionalChains, AfterSimulationCallback callback)
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

                callback(result, f, info, necessaryRensaPuyo);
            }            
        }
    }
}

template<typename T>
static void findPossibleRensasInternal(std::vector<T>& result, const Field& field, PuyoSet addedSet, int leftX, int restAdded)
{
    findRensasInternal(result, field, 0, [addedSet](vector<T>& result, const Field&, T& info, const NecessaryRensaPuyo& necessaryRensaPuyo) {
            info.necessaryPuyoSet.add(addedSet);
            info.necessaryPuyoSet.add(necessaryRensaPuyo.c, necessaryRensaPuyo.numNecessaryPuyos);
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

void RensaDetector::findPossibleRensasUsingIteration(std::vector<PossibleRensaInfo>& result, const Field& field, int maxIteration, int additionalChains, PuyoSet addedSet)
{
    if (maxIteration <= 0)
        return;

    findRensasInternal(result, field, additionalChains,
                       [addedSet, maxIteration](vector<PossibleRensaInfo>& result, const Field& f, PossibleRensaInfo& info, const NecessaryRensaPuyo& necessaryRensaPuyo) {
            info.necessaryPuyoSet.add(addedSet);
            info.necessaryPuyoSet.add(necessaryRensaPuyo.c, necessaryRensaPuyo.numNecessaryPuyos);
            result.push_back(info);
            RensaDetector::findPossibleRensasUsingIteration(result, f, maxIteration - 1, info.rensaInfo.chains, info.necessaryPuyoSet);
    });
}

void RensaDetector::findFeasibleRensas(vector<FeasibleRensaInfo>& result, const Field& field, int numKumiPuyo, const vector<KumiPuyo>& kumiPuyos)
{
    vector<Plan> plans;
    plans.reserve(22 + 22*22 + 22*22*22);
    findAvailablePlans(field, numKumiPuyo, kumiPuyos, plans);

    for (vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        if (!it->isRensaPlan())
            continue;

        result.push_back(FeasibleRensaInfo(BasicRensaInfo(it->totalChains(),
                                                          it->totalScore(),
                                                          it->totalFrames() - it->initiatingFrames()),
                                           it->initiatingFrames()));
    }
}

