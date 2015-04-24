#include "rensa_detector.h"

#include <glog/logging.h>

#include <algorithm>
#include <cstddef>
#include <string>

#include "base/base.h"
#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_bit_field.h"
#include "core/position.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"

using namespace std;

namespace {

const int EXTENTIONS[][3][2] = {
    // I
    {{ 0, 1}, { 0, 2}, { 0, 3}},
    {{-3, 0}, {-2, 0}, {-1, 0}},
    {{-2, 0}, {-1, 0}, { 1, 0}},
    {{-1, 0}, { 1, 0}, { 2, 0}},
    {{ 1, 0}, { 2, 0}, { 3, 0}},
    // O
    {{ 0, 1}, {-1, 0}, {-1, 1}},
    {{ 0, 1}, { 1, 0}, { 1, 1}},
    // Z
    {{ 0, 1}, { 1, 1}, { 1, 2}},
    {{ 0, 1}, {-1,-1}, {-1, 0}},
    {{ 1,-1}, { 1, 0}, { 2,-1}},
    {{ 0, 1}, {-1, 1}, { 1, 0}},
    {{-2, 1}, {-1, 0}, {-1, 1}},
    // S
    {{ 0, 1}, { 1,-1}, { 1, 0}},
    {{ 0, 1}, {-1, 1}, {-1, 2}},
    {{ 1, 0}, { 1, 1}, { 2, 1}},
    {{-1, 0}, { 0, 1}, { 1, 1}},
    {{-2,-1}, {-1,-1}, {-1, 0}},
    // J
    {{ 1, 0}, { 1, 1}, { 1, 2}},
    {{-1, 0}, { 0, 1}, { 0, 2}},
    {{ 0, 1}, { 1, 0}, { 2, 0}},
    {{-1, 0}, {-1, 1}, { 1, 0}},
    {{-2, 0}, {-2, 1}, {-1, 0}},
    {{ 0, 1}, { 0, 2}, { 1, 2}},
    {{-1,-2}, {-1,-1}, {-1, 0}},
    {{ 1, 0}, { 2,-1}, { 2, 0}},
    {{-1, 0}, { 1,-1}, { 1, 0}},
    {{-2, 1}, {-1, 1}, { 0, 1}},
    // L
    {{ 0, 1}, { 0, 2}, { 1, 0}},
    {{-1, 0}, {-1, 1}, {-1, 2}},
    {{ 0, 1}, { 1, 1}, { 2, 1}},
    {{-1,-1}, {-1, 0}, { 1, 0}},
    {{-2,-1}, {-2, 0}, {-1, 0}},
    {{ 1,-2}, { 1,-1}, { 1, 0}},
    {{-1, 2}, { 0, 1}, { 0, 2}},
    {{ 1, 0}, { 2, 0}, { 2, 1}},
    {{-1, 0}, { 1, 0}, { 1, 1}},
    {{-2, 0}, {-1, 0}, { 0, 1}},
    // T
    {{ 1, 0}, { 1, 1}, { 2, 0}},
    {{-1, 0}, { 0, 1}, { 1, 0}},
    {{-2, 0}, {-1, 0}, {-1, 1}},
    {{ 0, 1}, { 0, 2}, { 1, 1}},
    {{-1,-1}, {-1, 0}, {-1, 1}},
    {{ 1,-1}, { 1, 0}, { 2, 0}},
    {{-1, 1}, { 0, 1}, { 1, 1}},
    {{-2, 0}, {-1,-1}, {-1, 0}},
    {{ 1,-1}, { 1, 0}, { 1, 1}},
    {{-1, 1}, { 0, 1}, { 0, 2}},
};

}  // namespace anomymous

// static
void RensaDetector::makeProhibitArray(const RensaResult& rensaResult, const RensaChainTrackResult& trackResult,
                                      const CoreField& originalField, const ColumnPuyoList& firePuyos,
                                      bool prohibits[FieldConstant::MAP_WIDTH])
{
    std::fill(prohibits, prohibits + FieldConstant::MAP_WIDTH, true);

    for (int x = 1; x <= 6; ++x) {
        for (int y = originalField.height(x); y >= 1; --y) {
            if (trackResult.erasedAt(x, y) != rensaResult.chains)
                continue;

            if (originalField.color(x, y + 1) == PuyoColor::EMPTY || trackResult.erasedAt(x, y + 1) == rensaResult.chains) {
                prohibits[x] = false;
                // needs to check more.
                // e.g. Let's consider the following field.
                // A
                // B
                // C
                // and A and C are erased. B will drop, so we need to check it.
            } else {
                prohibits[x - 1] = false;
                prohibits[x] = false;
                prohibits[x + 1] = false;
                ++x; // x will advance by 2 in total.
                break;
            }
        }
    }

    for (int x = 1; x <= 6; ++x)
        prohibits[x] = (firePuyos.sizeOn(x) > 0);
}

// TODO(mayah): Consider to improve this.
static inline
void makeProhibitArrayForExtend(const RensaResult& /*rensaResult*/, const RensaChainTrackResult& /*trackResult*/,
                                const CoreField& /*originalField*/, const ColumnPuyoList& firePuyos,
                                bool prohibits[FieldConstant::MAP_WIDTH])
{
    std::fill(prohibits, prohibits + FieldConstant::MAP_WIDTH, false);
    for (int x = 1; x <= 6; ++x)
        prohibits[x] = (firePuyos.sizeOn(x) > 0);
}

// tryDropFire complements puyos in |originalField|, and fires a rensa.
// The complemented puyos are always grounded (This is the different point of tryFloatFire).
// For each detected rensa, |callback| is called.
static inline
void tryDropFire(const CoreField& originalField, const bool prohibits[FieldConstant::MAP_WIDTH],
                 PurposeForFindingRensa purpose, int maxComplementPuyos, int maxPuyoHeight,
                 const RensaDetector::SimulationCallback& callback)
{
    bool visited[CoreField::MAP_WIDTH][NUM_PUYO_COLORS] {};

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = originalField.height(x); y >= 1; --y) {
            PuyoColor c = originalField.color(x, y);

            if (!isNormalColor(c))
                continue;

            // Drop puyo on
            for (int d = -1; d <= 1; ++d) {
                if (prohibits[x + d])
                    continue;

                if (visited[x + d][ordinal(c)])
                    continue;

                if (x + d <= 0 || CoreField::WIDTH < x + d)
                    continue;
                if (d == 0) {
                    if (originalField.color(x, y + 1) != PuyoColor::EMPTY)
                        continue;

                    // If the first rensa is this, any rensa won't continue.
                    // This is like erasing the following X.
                    // ......
                    // .YXY..
                    // BZZZBB
                    // CAAACC
                    //
                    // So, we should be able to skip this.
                    if (purpose == PurposeForFindingRensa::FOR_FIRE && !originalField.isConnectedPuyo(x, y))
                        continue;
                } else {
                    if (originalField.color(x + d, y) != PuyoColor::EMPTY)
                        continue;
                }

                visited[x + d][ordinal(c)] = true;

                CoreField f(originalField);
                int necessaryPuyos = 0;

                bool ok = true;
                while (true) {
                    if (!f.dropPuyoOnWithMaxHeight(x + d, c, maxPuyoHeight)) {
                        ok = false;
                        break;
                    }

                    ++necessaryPuyos;

                    if (maxComplementPuyos < necessaryPuyos) {
                        ok = false;
                        break;
                    }
                    if (f.countConnectedPuyosMax4(x + d, f.height(x + d)) >= 4)
                        break;
                }

                if (!ok)
                    continue;

                ColumnPuyoList cpl;
                if (!cpl.add(x + d, c, necessaryPuyos))
                    continue;

                callback(&f, cpl);
            }
        }
    }
}

static inline
void tryFloatFire(const CoreField& originalField, const bool prohibits[FieldConstant::MAP_WIDTH],
                  int maxComplementPuyos, int maxPuyoHeight,
                  const RensaDetector::SimulationCallback& callback)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = std::min(12, originalField.height(x)); y >= 1; --y) {
            PuyoColor c = originalField.color(x, y);

            DCHECK_NE(c, PuyoColor::EMPTY);
            if (c == PuyoColor::OJAMA)
                continue;

            int necessaryPuyos = 4 - originalField.countConnectedPuyosMax4(x, y);
            if (necessaryPuyos > maxComplementPuyos)
                continue;

            // float puyo col dx
            for (int dx = x - 1; dx <= x + 1; ++dx) {
                if (dx <= 0 || CoreField::WIDTH < dx)
                    continue;
                if (prohibits[dx])
                    continue;
                if (x != dx && originalField.color(dx, y) != PuyoColor::EMPTY)
                    continue;

                int restPuyos = necessaryPuyos;
                CoreField f(originalField);
                ColumnPuyoList cpl;

                bool ok = true;
                while (f.height(dx) + restPuyos < y) {
                    if (!f.dropPuyoOnWithMaxHeight(dx, PuyoColor::OJAMA, maxPuyoHeight)) {
                        ok = false;
                        break;
                    }
                    if (!cpl.add(dx, PuyoColor::OJAMA)) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;

                while (restPuyos-- > 0) {
                    if (!f.dropPuyoOnWithMaxHeight(dx, c, maxPuyoHeight)) {
                        ok = false;
                        break;
                    }
                    if (!cpl.add(dx, c)) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;

                callback(&f, cpl);
            }
        }
    }
}

static inline
void tryExtendFire(const CoreField& originalField, const bool prohibits[FieldConstant::MAP_WIDTH],
                   int maxComplementPuyos, int maxPuyoHeight,
                   const RensaDetector::SimulationCallback& callback)
{
    FieldBitField checked;
    Position positions[FieldConstant::HEIGHT * FieldConstant::WIDTH];
    int working[FieldConstant::HEIGHT * FieldConstant::WIDTH];

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = std::min(12, originalField.height(x)); y >= 1; --y) {
            PuyoColor c = originalField.color(x, y);
            if (!isNormalColor(c))
                continue;
            if (checked(x, y))
                continue;
            if (!originalField.hasEmptyNeighbor(x, y))
                continue;
            Position* const head = originalField.fillSameColorPosition(x, y, c, positions, &checked);
            int size = head - positions;
            switch (size) {
            case 1: {
                Position origin = positions[0];
                for (size_t i = 0; i < ARRAY_SIZE(EXTENTIONS); ++i) {
                    bool ok = true;
                    for (int j = 0; j < 3; ++j) {
                        int xx = origin.x + EXTENTIONS[i][j][0];
                        int yy = origin.y + EXTENTIONS[i][j][1];
                        if (!(originalField.color(xx, yy) == PuyoColor::EMPTY || originalField.color(xx, yy) == c)) {
                            ok = false;
                            break;
                        }
                    }
                    if (!ok)
                        continue;

                    CoreField cf(originalField);
                    ColumnPuyoList cpl;
                    for (int j = 0; j < 3; ++j) {
                        int xx = origin.x + EXTENTIONS[i][j][0];
                        int yy = origin.y + EXTENTIONS[i][j][1];
                        if (cf.color(xx, yy) == c)
                            continue;
                        DCHECK_EQ(originalField.color(xx, yy), PuyoColor::EMPTY);
                        if (cf.color(xx, yy - 1) == PuyoColor::EMPTY) {
                            ok = false;
                            break;
                        }
                        if (prohibits[xx]) {
                            ok = false;
                            break;
                        }
                        if (!cf.dropPuyoOnWithMaxHeight(xx, c, maxPuyoHeight)) {
                            ok = false;
                            break;
                        }
                        cpl.add(xx, c);
                    }

                    if (!ok)
                        continue;

                    if (maxComplementPuyos < cpl.size())
                        continue;

                    callback(&cf, cpl);
                }
                break;
            }
            case 2: {
                Position origin;
                Position mustPosition;
                if (positions[0].x == positions[1].x) {
                    origin = Position(positions[0].x, std::min(positions[0].y, positions[1].y));
                    mustPosition = Position(positions[0].x, std::max(positions[0].y, positions[1].y));
                } else {
                    origin = Position(positions[0]);
                    mustPosition = Position(positions[1]);
                }

                for (size_t i = 0; i < ARRAY_SIZE(EXTENTIONS); ++i) {
                    bool ok = false;
                    for (int j = 0; j < 3; ++j) {
                        int xx = origin.x + EXTENTIONS[i][j][0];
                        int yy = origin.y + EXTENTIONS[i][j][1];
                        if (!(originalField.color(xx, yy) == PuyoColor::EMPTY || originalField.color(xx, yy) == c)) {
                            ok = false;
                            break;
                        }
                        if (Position(xx, yy) == mustPosition)
                            ok = true;
                    }

                    if (!ok)
                        continue;

                    CoreField cf(originalField);
                    ColumnPuyoList cpl;
                    for (int j = 0; j < 3; ++j) {
                        int xx = origin.x + EXTENTIONS[i][j][0];
                        int yy = origin.y + EXTENTIONS[i][j][1];
                        if (cf.color(xx, yy) == c)
                            continue;
                        DCHECK_EQ(originalField.color(xx, yy), PuyoColor::EMPTY);
                        if (cf.color(xx, yy - 1) == PuyoColor::EMPTY) {
                            ok = false;
                            break;
                        }
                        if (prohibits[xx]) {
                            ok = false;
                            break;
                        }
                        if (!cf.dropPuyoOnWithMaxHeight(xx, c, maxPuyoHeight)) {
                            ok = false;
                            break;
                        }
                        cpl.add(xx, c);
                    }

                    if (maxComplementPuyos < cpl.size())
                        continue;

                    if (!ok)
                        continue;

                    callback(&cf, cpl);
                }
                break;
            }
            case 3: {
                int pos = 0;
                for (Position* p = positions; p != head; ++p) {
                    if (originalField.color(p->x + 1, p->y) == PuyoColor::EMPTY)
                        working[pos++] = p->x + 1;
                    if (originalField.color(p->x - 1, p->y) == PuyoColor::EMPTY)
                        working[pos++] = p->x - 1;
                    if (originalField.color(p->x, p->y + 1) == PuyoColor::EMPTY)
                        working[pos++] = p->x;
                    if (originalField.color(p->x, p->y - 1) == PuyoColor::EMPTY)
                        working[pos++] = p->x;
                }
                std::sort(working, working + pos);
                int* endX = std::unique(working, working + pos);
                for (int i = 0; i < endX - working; ++i) {
                    int xx = working[i];
                    if (prohibits[xx])
                        continue;
                    CoreField cf(originalField);
                    if (!cf.dropPuyoOn(xx, c))
                        continue;
                    if (maxPuyoHeight < cf.height(xx))
                        continue;
                    ColumnPuyoList cpl;
                    if (!cpl.add(working[i], c))
                        continue;
                    callback(&cf, cpl);
                }
                break;
            }
            default:
                CHECK(false) << size << '\n' << originalField.toDebugString();
            }
        }
    }
}

static inline void findRensas(const CoreField& field,
                              const RensaDetectorStrategy& strategy,
                              const bool prohibits[FieldConstant::MAP_WIDTH],
                              PurposeForFindingRensa purpose,
                              const RensaDetector::SimulationCallback& callback)
{
    int maxPuyoHeight = 12;
    int complementPuyos;
    switch (purpose) {
    case PurposeForFindingRensa::FOR_KEY:
        complementPuyos = strategy.maxNumOfComplementPuyosForKey();
        if (strategy.allowsPuttingKeyPuyoOn13thRow())
            maxPuyoHeight = 13;
        break;
    case PurposeForFindingRensa::FOR_FIRE:
        complementPuyos = strategy.maxNumOfComplementPuyosForFire();
        break;
    default:
        CHECK(false);
    }

    switch (strategy.mode()) {
    case RensaDetectorStrategy::Mode::DROP:
        tryDropFire(field, prohibits, purpose, complementPuyos, maxPuyoHeight, callback);
        break;
    case RensaDetectorStrategy::Mode::FLOAT:
        tryFloatFire(field, prohibits, complementPuyos, maxPuyoHeight, callback);
        break;
    case RensaDetectorStrategy::Mode::EXTEND:
        tryExtendFire(field, prohibits, complementPuyos, maxPuyoHeight, callback);
        break;
    default:
        CHECK(false) << "Unknown mode : " << static_cast<int>(strategy.mode());
    }
}

void RensaDetector::detect(const CoreField& original,
                           const RensaDetectorStrategy& strategy,
                           PurposeForFindingRensa purpose,
                           const bool prohibits[FieldConstant::MAP_WIDTH],
                           const RensaDetector::SimulationCallback& callback)
{
    findRensas(original, strategy, prohibits, purpose, callback);
}

template<typename Callback>
static void findPossibleRensasInternal(const CoreField& currentField,
                                       const ColumnPuyoList& keyPuyos,
                                       int leftX,
                                       int restAdded,
                                       PurposeForFindingRensa purpose,
                                       const RensaDetectorStrategy& strategy,
                                       const Callback& callback)
{
    auto findRensaCallback = [&currentField, &keyPuyos, &callback](CoreField* f, const ColumnPuyoList& firePuyos) {
        ColumnPuyoList whole(keyPuyos);
        whole.merge(firePuyos);
        callback(f, whole);
    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    findRensas(currentField, strategy, prohibits, purpose, findRensaCallback);

    if (restAdded <= 0)
        return;

    CoreField f(currentField);
    ColumnPuyoList puyoList(keyPuyos);

    for (int x = leftX; x <= CoreField::WIDTH; ++x) {
        if (f.height(x) >= 12)
            continue;

        for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
            PuyoColor c = NORMAL_PUYO_COLORS[i];

            if (!f.dropPuyoOn(x, c))
                continue;
            if (!puyoList.add(x, c))
                continue;

            if (f.countConnectedPuyosMax4(x, f.height(x)) < 4)
                findPossibleRensasInternal(f, puyoList, x, restAdded - 1, purpose, strategy, callback);

            f.removeTopPuyoFrom(x);
            puyoList.removeTopFrom(x);
        }
    }
}

void RensaDetector::iteratePossibleRensas(const CoreField& originalField,
                                          int maxKeyPuyos,
                                          const RensaDetectorStrategy& strategy,
                                          const RensaDetector::RensaCallback& callback)
{
    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));

    auto cb = [&originalContext, &callback](CoreField* cf, const ColumnPuyoList& cpl) {
        CoreField::SimulationContext context(originalContext);
        RensaResult rensaResult = cf->simulate(&context);
        if (rensaResult.chains > 0)
            callback(*cf, rensaResult, cpl);
    };

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(originalField, puyoList, 1, maxKeyPuyos, PurposeForFindingRensa::FOR_FIRE, strategy, cb);
}

void RensaDetector::iteratePossibleRensasWithTracking(const CoreField& originalField,
                                                      int maxKeyPuyos,
                                                      const RensaDetectorStrategy& strategy,
                                                      const RensaDetector::TrackedPossibleRensaCallback& callback)
{
    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));
    auto cb = [&originalContext, &callback](CoreField* cf, const ColumnPuyoList& cpl) {
        CoreField::SimulationContext context(originalContext);
        RensaChainTracker tracker;
        RensaResult rensaResult = cf->simulate(&context, &tracker);
        if (rensaResult.chains > 0)
            callback(*cf, rensaResult, cpl, tracker.result());
    };

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(originalField, puyoList, 1, maxKeyPuyos, PurposeForFindingRensa::FOR_FIRE, strategy, cb);
}

void RensaDetector::iteratePossibleRensasWithCoefTracking(const CoreField& originalField,
                                                          int maxKeyPuyos,
                                                          const RensaDetectorStrategy& strategy,
                                                          const RensaDetector::CoefPossibleRensaCallback& callback)
{
    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));
    auto cb = [&originalContext, &callback](CoreField* cf, const ColumnPuyoList& cpl) {
        CoreField::SimulationContext context(originalContext);
        RensaCoefTracker tracker;
        RensaResult rensaResult = cf->simulate(&context, &tracker);
        if (rensaResult.chains > 0)
            callback(*cf, rensaResult, cpl, tracker.result());
    };

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(originalField, puyoList, 1, maxKeyPuyos, PurposeForFindingRensa::FOR_FIRE, strategy, cb);
}

void RensaDetector::iteratePossibleRensasWithVanishingPositionTracking(const CoreField& originalField,
                                                                       int maxKeyPuyos,
                                                                       const RensaDetectorStrategy& strategy,
                                                                       const RensaDetector::VanishingPositionPossibleRensaCallback& callback)
{
    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));
    auto cb = [&originalContext, &callback](CoreField* cf, const ColumnPuyoList& cpl) {
        CoreField::SimulationContext context(originalContext);
        RensaVanishingPositionTracker tracker;
        RensaResult rensaResult = cf->simulate(&context, &tracker);
        if (rensaResult.chains > 0)
            callback(*cf, rensaResult, cpl, tracker.result());
    };

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(originalField, puyoList, 1, maxKeyPuyos, PurposeForFindingRensa::FOR_FIRE, strategy, cb);
}

static
void iteratePossibleRensasIterativelyInternal(const CoreField& currentField,
                                              const CoreField& originalField,
                                              int restIterations,
                                              const ColumnPuyoList& accumulatedKeyPuyos,
                                              const ColumnPuyoList& firstRensaFirePuyos,
                                              int currentTotalChains,
                                              const bool prohibits[FieldConstant::MAP_WIDTH],
                                              const RensaDetectorStrategy& strategy,
                                              const RensaDetector::TrackedPossibleRensaCallback& callback)
{
    if (restIterations <= 0)
        return;

    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));
    const CoreField::SimulationContext currentContext(CoreField::SimulationContext::fromField(currentField));

    auto findRensaCallback = [&](CoreField* fieldComplemented, const ColumnPuyoList& currentFirePuyos) {
        int additionalChains = 0;
        {
            CoreField::SimulationContext context(currentContext);
            RensaResult rensaResult = fieldComplemented->simulate(&context);
            if (rensaResult.chains == 0)
                return;
            additionalChains = rensaResult.chains;
        }

        ColumnPuyoList combinedKeyPuyos(accumulatedKeyPuyos);
        if (!combinedKeyPuyos.merge(currentFirePuyos))
            return;

        int maxHeight = strategy.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

        // Here, try to fire the combined rensa.
        CoreField cf(originalField);
        if (!cf.dropPuyoListWithMaxHeight(combinedKeyPuyos, maxHeight))
            return;

        // Check putting key puyo does not fire a rensa. Rensa should not start when we add key puyos.
        if (cf.rensaWillOccurWithContext(originalContext))
            return;

        // Since key puyo does not fire a rensa, we can safely include the key puyos in context.
        CoreField::SimulationContext context = CoreField::SimulationContext::fromField(cf);

        // Then, fire a rensa.
        if (!cf.dropPuyoListWithMaxHeight(firstRensaFirePuyos, maxHeight))
            return;

        RensaChainTracker tracker;
        RensaResult combinedRensaResult = cf.simulate(&context, &tracker);
        const RensaChainTrackResult& combinedTrackResult = tracker.result();

        if (combinedRensaResult.chains != currentTotalChains + additionalChains) {
            // Rensa looks broken. We don't count such rensa.
            return;
        }

        // Don't put key puyo on the column which fire puyo will be placed.
        bool newProhibits[FieldConstant::MAP_WIDTH];
        if (strategy.mode() == RensaDetectorStrategy::Mode::EXTEND)
            makeProhibitArrayForExtend(combinedRensaResult, combinedTrackResult, currentField, firstRensaFirePuyos, newProhibits);
        else
            RensaDetector::makeProhibitArray(combinedRensaResult, combinedTrackResult, currentField, firstRensaFirePuyos, newProhibits);

        ColumnPuyoList wholeColumnPuyoList(combinedKeyPuyos);
        wholeColumnPuyoList.merge(firstRensaFirePuyos);
        callback(cf, combinedRensaResult, wholeColumnPuyoList, combinedTrackResult);
        iteratePossibleRensasIterativelyInternal(cf, originalField, restIterations - 1,
                                                 combinedKeyPuyos, firstRensaFirePuyos,
                                                 combinedRensaResult.chains,
                                                 newProhibits,
                                                 strategy, callback);
    };

    findRensas(currentField, strategy, prohibits, PurposeForFindingRensa::FOR_KEY, findRensaCallback);
}

// iteratePossibleRensasIteratively finds rensa with the following algorithm.
// 1. First, iterate all rensas (first rensa) from the current field.
// 2. For each field after the rensa, we find another rensa (second rensa).
// 3. We think the necessary puyos to fire the second rensa is the key puyos for the first rensa.
//    Add the key puyos to the original field, and try to fire a rensa. If the first rensa and the second
//    rensa can be combined, we think the combined rensa as the whole rensa.
//    Otherwise, we think it is broken.
void RensaDetector::iteratePossibleRensasIteratively(const CoreField& originalField,
                                                     int maxIteration,
                                                     const RensaDetectorStrategy& strategy,
                                                     const TrackedPossibleRensaCallback& callback)
{
    DCHECK_LE(1, maxIteration);

    const CoreField::SimulationContext originalContext(CoreField::SimulationContext::fromField(originalField));

    auto findRensaCallback = [&](CoreField* f, const ColumnPuyoList& firePuyos) {
        CoreField::SimulationContext context(originalContext);
        RensaChainTracker tracker;
        RensaResult rensaResult = f->simulate(&context, &tracker);
        if (rensaResult.chains == 0)
            return;

        const RensaChainTrackResult& trackResult = tracker.result();
        callback(*f, rensaResult, firePuyos, trackResult);

        // Don't put key puyo on the column which fire puyo will be placed.
        bool prohibits[FieldConstant::MAP_WIDTH] {};
        if (strategy.mode() == RensaDetectorStrategy::Mode::EXTEND)
            makeProhibitArrayForExtend(rensaResult, trackResult, originalField, firePuyos, prohibits);
        else
            makeProhibitArray(rensaResult, trackResult, originalField, firePuyos, prohibits);

        iteratePossibleRensasIterativelyInternal(*f, originalField, maxIteration - 1,
                                                 ColumnPuyoList(), firePuyos, rensaResult.chains, prohibits, strategy, callback);
    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    findRensas(originalField, strategy, prohibits, PurposeForFindingRensa::FOR_FIRE, findRensaCallback);
}
