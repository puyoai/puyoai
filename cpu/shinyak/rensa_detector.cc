#include "rensa_detector.h"

#include "field.h"
#include "plan.h"

using namespace std;

template<typename T>
struct AfterSimulationCallback_findRensas {
    void operator()(vector<T>& result, const Field&, const T& info) const {
        result.push_back(info);
    }
};

template<typename AfterSimulationCallback, typename T>
static void findRensasInternal(vector<T>& result, const Field& field, int additionalChains, const PuyoSet& puyoSet, AfterSimulationCallback callback)
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

                T info;
                info.necessaryPuyoSet.add(puyoSet);
                info.necessaryPuyoSet.add(c, necessaryPuyos);
                f.simulate(info.rensaInfo, additionalChains);

                callback(result, f, info);
            }            
        }
    }
}

template<typename T>
static void findPossibleRensasInternal(std::vector<T>& result, const Field& field, PuyoSet addedSet, int leftX, int restAdded)
{
    findRensasInternal(result, field, 0, addedSet, AfterSimulationCallback_findRensas<T>());

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


struct AfterSimulationCallback_findPossibleRensasUsingIteration {
    explicit AfterSimulationCallback_findPossibleRensasUsingIteration(int restIteration)
        : restIteration(restIteration) {}

    void operator()(vector<PossibleRensaInfo>& result, const Field& f, const PossibleRensaInfo& info) const {
        result.push_back(info);
        RensaDetector::findPossibleRensasUsingIteration(result, f, restIteration, info.rensaInfo.chains, info.necessaryPuyoSet);
    }

    int restIteration;
};

void RensaDetector::findPossibleRensasUsingIteration(std::vector<PossibleRensaInfo>& result, const Field& field, int maxIteration, int additionalChains, PuyoSet puyoSet)
{
    if (maxIteration <= 0)
        return;

    findRensasInternal(result, field, additionalChains, puyoSet, AfterSimulationCallback_findPossibleRensasUsingIteration(maxIteration - 1));
}

void RensaDetector::findFeasibleRensas(vector<FeasibleRensaInfo>& result, const Field& field, int numKumiPuyo, const vector<KumiPuyo>& kumiPuyos)
{
    vector<Plan> plans;
    plans.reserve(22 + 22*22 + 22*22*22);
    field.findAvailablePlans(numKumiPuyo, kumiPuyos, plans);

    for (vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        if (!it->isRensaPlan())
            continue;

        result.push_back(FeasibleRensaInfo(BasicRensaInfo(it->totalChains(),
                                                          it->totalScore(),
                                                          it->totalFrames() - it->initiatingFrames()),
                                           it->initiatingFrames()));
    }
}

