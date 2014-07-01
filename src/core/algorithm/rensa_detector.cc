#include "rensa_detector.h"

#include <iostream>
#include <set>
#include <vector>

#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_info.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

using namespace std;

std::vector<FeasibleRensaInfo>
RensaDetector::findFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    std::vector<FeasibleRensaInfo> result;
    Plan::iterateAvailablePlans(field, kumipuyoSeq, kumipuyoSeq.size(), [&result](const RefPlan& plan) {
        if (plan.isRensaPlan())
            result.emplace_back(plan.rensaResult(), plan.initiatingFrames());
    });

    return result;
}

static inline void simulateInternal(CoreField* f, const CoreField& original, PossibleRensaInfo* info)
{
    int minHeights[CoreField::MAP_WIDTH] {
        100, original.height(1) + 1, original.height(2) + 1, original.height(3) + 1,
        original.height(4) + 1, original.height(5) + 1, original.height(6) + 1, 100,
    };
    info->rensaResult = f->simulateWithMinHeights(minHeights);
}

static inline void simulateInternal(CoreField* f, const CoreField& original, TrackedPossibleRensaInfo* info)
{
    int minHeights[CoreField::MAP_WIDTH] {
        100, original.height(1) + 1, original.height(2) + 1, original.height(3) + 1,
        original.height(4) + 1, original.height(5) + 1, original.height(6) + 1, 100,
    };
    info->rensaResult = f->simulateAndTrackWithMinHeights(&info->trackResult, minHeights);
}

template<typename T>
static void findPossibleRensasInternal(const CoreField& field,
                                       const ColumnPuyoList& addedPuyos,
                                       int leftX,
                                       int restAdded,
                                       RensaDetector::Mode mode,
                                       std::vector<T>* result)
{
    RensaDetector::findRensas(field, mode,
                              [result, addedPuyos](CoreField* f, const CoreField& originalField, int x, PuyoColor c, int n) {
            // Making T info is a bit heavy. So, we make directly in vector, and use its instance.
            // This improves performance much.
            result->emplace_back();
            T& info = result->back();
            simulateInternal(f, originalField, &info);
            info.necessaryPuyoSet = addedPuyos;
            for (int i = 0; i < n; ++i)
                info.necessaryPuyoSet.addPuyo(x, c);
    });

    if (restAdded <= 0)
        return;

    CoreField f(field);
    ColumnPuyoList puyoList(addedPuyos);

    for (int x = leftX; x <= CoreField::WIDTH; ++x) {
        if (f.height(x) >= 13)
            continue;

        for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
            PuyoColor c = normalPuyoColorOf(i);

            f.dropPuyoOn(x, c);
            puyoList.addPuyo(x, c);

            if (f.countConnectedPuyos(x, f.height(x)) < 4)
                findPossibleRensasInternal(f, puyoList, x, restAdded - 1, mode, result);

            f.removeTopPuyoFrom(x);
            puyoList.removeLastAddedPuyo();
        }
    }
}

std::vector<PossibleRensaInfo>
RensaDetector::findPossibleRensas(const CoreField& field, int maxKeyPuyos, Mode mode)
{
    std::vector<PossibleRensaInfo> result;
    result.reserve(100000);

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(field, puyoList, 1, maxKeyPuyos, mode, &result);
    return result;
}

std::vector<TrackedPossibleRensaInfo>
RensaDetector::findPossibleRensasWithTracking(const CoreField& field, int maxKeyPuyos, Mode mode)
{
    std::vector<TrackedPossibleRensaInfo> result;
    result.reserve(100000);

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(field, puyoList, 1, maxKeyPuyos, mode, &result);
    return result;
}

static inline void tryDropFire(const CoreField& originalField, RensaDetector::SimulationCallback callback)
{
    bool visited[CoreField::WIDTH][NUM_PUYO_COLORS] {};

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = originalField.height(x); y >= 1; --y) {
            PuyoColor c = originalField.color(x, y);

            if (!isNormalColor(c))
                continue;

            // Drop puyo on
            for (int d = -1; d <= 1; ++d) {

                if (visited[x + d][c])
                    continue;
                visited[x + d][c] = true;

                if (x + d <= 0 || CoreField::WIDTH < x + d)
                    continue;
                if (d == 0) {
                    if (originalField.color(x, y + 1) != PuyoColor::EMPTY)
                        continue;
                } else {
                    if (originalField.color(x + d, y) != PuyoColor::EMPTY)
                        continue;
                }

                CoreField f(originalField);
                int necessaryPuyos = 0;
                while (necessaryPuyos <= 4 && f.countConnectedPuyos(x, y) < 4 && f.height(x + d) <= 13) {
                    f.dropPuyoOn(x + d, c);
                    ++necessaryPuyos;
                }

                if (necessaryPuyos > 4)
                    continue;

                callback(&f, originalField, x + d, c, necessaryPuyos);
            }
        }
    }
}

static inline void tryFloatFire(const CoreField& originalField, RensaDetector::SimulationCallback callback)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = originalField.height(x); y >= 1; --y) {
            PuyoColor c = originalField.color(x, y);

            DCHECK(c != PuyoColor::EMPTY);
            if (c == PuyoColor::OJAMA)
                continue;

            int necessaryPuyos = 4 - originalField.countConnectedPuyos(x, y);
            int restPuyos = necessaryPuyos;
            CoreField f(originalField);

            int dx = x - 1;
            // float puyo col dx
            for (; dx <= x + 1 && restPuyos > 0; ++dx) {
                if (dx <= 0 || CoreField::WIDTH < dx) {
                    continue;
                }


                // Check y
                if (dx != x) {
                    if (originalField.color(dx, y) != PuyoColor::EMPTY) {
                        continue;
                    } else { // restPuyos must be more than 0
                        f.unsafeSet(dx, y, c);
                        --restPuyos;
                    }
                }

                int dy_min = y - 1;
                // Check under y
                for (; restPuyos > 0 && dy_min > 0 && originalField.color(dx ,dy_min) == PuyoColor::EMPTY;
                     --dy_min) {
                    f.unsafeSet(dx, dy_min, c);
                    --restPuyos;
                }

                // Check over y
                for (int dy = y + 1;
                     restPuyos > 0 && dy <= 12 && originalField.color(dx ,dy) == PuyoColor::EMPTY; ++dy) {
                    f.unsafeSet(dx, dy, c);
                    --restPuyos;
                }

                // Fill ojama
                for(; dy_min > 0 && originalField.color(dx, dy_min) == PuyoColor::EMPTY; --dy_min) {
                    f.unsafeSet(dx, dy_min, PuyoColor::OJAMA);
                }

                f.recalcHeightOn(dx);
            }

            if (restPuyos <= 0) {
                callback(&f, originalField, dx, c, necessaryPuyos);
            }
        }
    }
}

void RensaDetector::findRensas(const CoreField& field, RensaDetector::Mode mode, RensaDetector::SimulationCallback callback)
{
    switch (mode) {
    case Mode::DROP:
        tryDropFire(field, callback);
        break;
    case Mode::FLOAT:
        tryFloatFire(field, callback);
        break;
    default:
        CHECK(false) << "Unknown mode : " << static_cast<int>(mode);
    }
}
