#include "rensa_detector.h"

#include <glog/logging.h>

#include <algorithm>
#include <cstddef>
#include <string>

#include "base/base.h"
#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_checker.h"
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

// detectByDropStrategy complements puyos in |originalField|, and fires a rensa.
// The complemented puyos are always grounded (This is the different point of tryFloatFire).
// For each detected rensa, |callback| is called.
// static
void RensaDetector::detectByDropStrategy(const CoreField& originalField,
                                         const bool prohibits[FieldConstant::MAP_WIDTH],
                                         PurposeForFindingRensa purpose,
                                         int maxComplementPuyos,
                                         int maxPuyoHeight,
                                         const RensaDetector::ComplementCallback& callback)
{
    bool visited[FieldConstant::MAP_WIDTH][NUM_PUYO_COLORS] {};

    FieldBits normalColorBits = originalField.bitField().normalColorBits();
    FieldBits emptyBits = originalField.bitField().bits(PuyoColor::EMPTY);

    FieldBits edgeBits = (normalColorBits & emptyBits.expandEdge()).maskedField12();

    edgeBits.iterateBitPositions([&](int x, int y) {
        DCHECK(originalField.isNormalColor(x, y));

        PuyoColor c = originalField.color(x, y);

        // Drop puyo on
        for (int d = -1; d <= 1; ++d) {
            if (prohibits[x + d])
                continue;

            if (visited[x + d][ordinal(c)])
                continue;

            if (x + d <= 0 || FieldConstant::WIDTH < x + d)
                continue;
            if (d == 0) {
                if (!originalField.isEmpty(x, y + 1))
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
                if (!originalField.isEmpty(x + d, y))
                    continue;
            }

            visited[x + d][ordinal(c)] = true;

            int necessaryPuyos = 0;

            bool ok = true;
            CoreField cf(originalField);
            while (true) {
                if (!cf.dropPuyoOnWithMaxHeight(x + d, c, maxPuyoHeight)) {
                    ok = false;
                    break;
                }

                ++necessaryPuyos;

                if (maxComplementPuyos < necessaryPuyos) {
                    ok = false;
                    break;
                }
                if (cf.countConnectedPuyosMax4(x + d, cf.height(x + d), c) >= 4)
                    break;
            }

            if (!ok)
                continue;

            ColumnPuyoList cpl;
            if (!cpl.add(x + d, c, necessaryPuyos))
                continue;

            callback(std::move(cf), cpl);
        }
    });
}

// static
void RensaDetector::detectByFloatStrategy(const CoreField& originalField,
                                          const bool prohibits[FieldConstant::MAP_WIDTH],
                                          int maxComplementPuyos,
                                          int maxPuyoHeight,
                                          const RensaDetector::ComplementCallback& callback)
{
    FieldBits normalColorBits = originalField.bitField().normalColorBits();
    FieldBits emptyBits = originalField.bitField().bits(PuyoColor::EMPTY);

    FieldBits edgeBits = (normalColorBits & emptyBits.expandEdge()).maskedField12();

    edgeBits.iterateBitPositions([&](int x, int y) {
        DCHECK(originalField.isNormalColor(x, y));

        int necessaryPuyos = 4 - originalField.countConnectedPuyosMax4(x, y);
        if (necessaryPuyos > maxComplementPuyos)
            return;

        PuyoColor c = originalField.color(x, y);

        // float puyo col dx
        for (int dx = x - 1; dx <= x + 1; ++dx) {
            if (dx <= 0 || FieldConstant::WIDTH < dx)
                continue;
            if (prohibits[dx])
                continue;
            if (x != dx && !originalField.isEmpty(dx, y))
                continue;

            CoreField cf(originalField);

            int restPuyos = necessaryPuyos;
            ColumnPuyoList cpl;

            bool ok = true;
            while (cf.height(dx) + restPuyos < y) {
                if (!cf.dropPuyoOnWithMaxHeight(dx, PuyoColor::OJAMA, maxPuyoHeight)) {
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
                if (!cf.dropPuyoOnWithMaxHeight(dx, c, maxPuyoHeight)) {
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

            callback(std::move(cf), cpl);
        }
    });
}

// static
void RensaDetector::detectByExtendStrategy(const CoreField& originalField,
                                           const bool prohibits[FieldConstant::MAP_WIDTH],
                                           int maxComplementPuyos,
                                           int maxPuyoHeight,
                                           const RensaDetector::ComplementCallback& callback)
{
    FieldBits checked;
    Position positions[FieldConstant::HEIGHT * FieldConstant::WIDTH];
    int working[FieldConstant::HEIGHT * FieldConstant::WIDTH];

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = std::min(12, originalField.height(x)); y >= 1; --y) {

            PuyoColor c = originalField.color(x, y);
            if (!isNormalColor(c))
                continue;
            if (checked.get(x, y))
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
                        if (xx < 1 || FieldConstant::WIDTH < x || yy < 1 || FieldConstant::HEIGHT < y) {
                            ok = false;
                            break;
                        }
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
                        if (xx < 1 || FieldConstant::WIDTH < x || yy < 1 || FieldConstant::HEIGHT < y) {
                            continue;
                        }
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
                        if (!cpl.add(xx, c)) {
                            ok = false;
                            break;
                        }
                    }

                    if (!ok)
                        continue;
                    if (maxComplementPuyos < cpl.size())
                        continue;

                    callback(std::move(cf), cpl);
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
                        if (xx < 1 || FieldConstant::WIDTH < x || yy < 1 || FieldConstant::HEIGHT < y) {
                            ok = false;
                            break;
                        }
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
                        if (xx < 1 || FieldConstant::WIDTH < x || yy < 1 || FieldConstant::HEIGHT < y)
                            continue;
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
                        if (!cpl.add(xx, c)) {
                            ok = false;
                            break;
                        }
                    }

                    if (!ok) {
                        continue;
                    }

                    if (maxComplementPuyos < cpl.size()) {
                        continue;
                    }

                    callback(std::move(cf), cpl);
                }
                break;
            }
            case 3: {
                int pos = 0;
                for (Position* p = positions; p != head; ++p) {
                    if (originalField.isEmpty(p->x + 1, p->y) && !originalField.isEmpty(p->x + 1, p->y - 1))
                        working[pos++] = p->x + 1;
                    if (originalField.isEmpty(p->x - 1, p->y) && !originalField.isEmpty(p->x - 1, p->y - 1))
                        working[pos++] = p->x - 1;
                    if (originalField.isEmpty(p->x, p->y + 1) && !originalField.isEmpty(p->x, p->y))
                        working[pos++] = p->x;
                    if (originalField.isEmpty(p->x, p->y - 1) && !originalField.isEmpty(p->x, p->y - 2))
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
                    callback(std::move(cf), cpl);
                }
                break;
            }
            default:
                CHECK(false) << size << '\n' << originalField.toDebugString();
            }
        }
    }
}

// static
void RensaDetector::detect(const CoreField& originalField,
                           const RensaDetectorStrategy& strategy,
                           PurposeForFindingRensa purpose,
                           const bool prohibits[FieldConstant::MAP_WIDTH],
                           const RensaDetector::ComplementCallback& callback)
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
        detectByDropStrategy(originalField, prohibits, purpose, complementPuyos, maxPuyoHeight, callback);
        break;
    case RensaDetectorStrategy::Mode::FLOAT:
        detectByFloatStrategy(originalField, prohibits, complementPuyos, maxPuyoHeight, callback);
        break;
    case RensaDetectorStrategy::Mode::EXTEND:
        detectByExtendStrategy(originalField, prohibits, complementPuyos, maxPuyoHeight, callback);
        break;
    default:
        CHECK(false) << "Unknown mode : " << static_cast<int>(strategy.mode());
    }
}

// static
void RensaDetector::detectSingle(const CoreField& cf,
                                 const RensaDetectorStrategy& strategy,
                                 const ComplementCallback& callback)
{
    const bool noProhibits[FieldConstant::MAP_WIDTH] {};
    detect(cf, strategy, PurposeForFindingRensa::FOR_FIRE, noProhibits, callback);
}

// static
void RensaDetector::detectIteratively(const CoreField& originalField,
                                      const RensaDetectorStrategy& strategy,
                                      int maxIteration,
                                      const RensaSimulationCallback& callback)
{
    DCHECK_LE(1, maxIteration);

    auto detectCallback = [&](CoreField&& complementedField, const ColumnPuyoList& firePuyos) {
        CoreField cf(complementedField);
        RensaLastVanishedPositionTracker tracker;

        int chains = cf.simulateFast(&tracker);
        if (chains == 0)
            return;

        (void)callback(std::move(complementedField), firePuyos);

        // Don't put key puyo on the column which fire puyo will be placed.
        bool prohibits[FieldConstant::MAP_WIDTH] {};
        makeProhibitArray(originalField, strategy, tracker.result(), firePuyos, prohibits);
        detectIterativelyInternal(originalField, strategy, cf, maxIteration - 1,
                                  ColumnPuyoList(), firePuyos, chains, prohibits, callback);
    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    detect(originalField, strategy, PurposeForFindingRensa::FOR_FIRE, prohibits, detectCallback);
}

// static
void RensaDetector::detectIterativelyInternal(const CoreField& originalField,
                                              const RensaDetectorStrategy& strategy,
                                              const CoreField& currentField,
                                              int restIterations,
                                              const ColumnPuyoList& accumulatedKeyPuyos,
                                              const ColumnPuyoList& firstRensaFirePuyos,
                                              int currentTotalChains,
                                              const bool prohibits[FieldConstant::MAP_WIDTH],
                                              const RensaSimulationCallback& callback)
{
    if (restIterations <= 0)
        return;

    auto detectCallback = [&](CoreField&& complementedField, const ColumnPuyoList& currentFirePuyos) {
        RensaLastVanishedPositionTracker tracker;
        int partialChains = complementedField.simulateFast(&tracker);
        if (partialChains == 0)
            return;

        RensaLastVanishedPositionTrackResult trackResult = tracker.result();

        ColumnPuyoList combinedKeyPuyos(accumulatedKeyPuyos);
        if (!combinedKeyPuyos.merge(currentFirePuyos))
            return;

        int maxHeight = strategy.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

        // Here, try to fire the combined rensa.
        CoreField cf(originalField);
        if (!cf.dropPuyoListWithMaxHeight(combinedKeyPuyos, maxHeight))
            return;

        // Check putting key puyo does not fire a rensa. Rensa should not start when we add key puyos.
        if (cf.rensaWillOccur())
            return;

        // Then, fire a rensa.
        if (!cf.dropPuyoListWithMaxHeight(firstRensaFirePuyos, maxHeight))
            return;

        ColumnPuyoList allComplemented(combinedKeyPuyos);
        allComplemented.merge(firstRensaFirePuyos);

        RensaResult combinedRensaResult = callback(std::move(cf), allComplemented);
        if (combinedRensaResult.chains != currentTotalChains + partialChains) {
            // Rensa looks broken. We don't count such rensa.
            return;
        }

        // Don't put key puyo on the column which fire puyo will be placed.
        bool newProhibits[FieldConstant::MAP_WIDTH];
        makeProhibitArray(originalField, strategy, trackResult, firstRensaFirePuyos, newProhibits);

        detectIterativelyInternal(originalField, strategy, complementedField,
                                  restIterations - 1, combinedKeyPuyos, firstRensaFirePuyos,
                                  combinedRensaResult.chains, newProhibits, callback);
    };

    detect(currentField, strategy, PurposeForFindingRensa::FOR_KEY, prohibits, detectCallback);
}

// static
void RensaDetector::detectSideChain(const CoreField& originalField,
                                    const RensaDetectorStrategy& strategy,
                                    const ComplementCallback& callback)
{
    const bool noProhibitedColumn[FieldConstant::MAP_WIDTH] {};

    auto detectCallback = [&](CoreField&& complementedField, const ColumnPuyoList& firePuyoList) {
        detectSideChainFromDetectedField(originalField, complementedField, strategy, firePuyoList, callback);
    };

    detect(originalField, strategy, PurposeForFindingRensa::FOR_FIRE, noProhibitedColumn, detectCallback);
}

// static
void RensaDetector::detectSideChainFromDetectedField(const CoreField& originalField,
                                                     const CoreField& fireDetectedField,
                                                     const RensaDetectorStrategy& strategy,
                                                     const ColumnPuyoList& firePuyoList,
                                                     const ComplementCallback& callback)
{
    CoreField detectedField(fireDetectedField);
    for (int i = 0; i < 2; ++i) {
        if (detectedField.vanishDrop().score == 0)
            return;

        bool prohibitedColumns[FieldConstant::MAP_WIDTH] {};
        std::fill(prohibitedColumns, prohibitedColumns + FieldConstant::MAP_WIDTH, true);
        for (int x = 1; x <= 6; ++x) {
            if (detectedField.height(x) == fireDetectedField.height(x))
                continue;
            prohibitedColumns[x] = false;
            prohibitedColumns[x - 1] = false;
            prohibitedColumns[x + 1] = false;
        }
        prohibitedColumns[0] = prohibitedColumns[FieldConstant::MAP_WIDTH - 1] = true;

        auto detectCallback = [&](CoreField&& complementedField, const ColumnPuyoList& keyPuyos) {
            // Check we have 2-rensa.
            if (complementedField.vanishDrop().score == 0)
                return;
            // Check we don't have 3-rensa.
            if (complementedField.vanishDrop().score > 0)
                return;

            ColumnPuyoList cpl(keyPuyos);
            cpl.merge(firePuyoList);
            CoreField cf(originalField);
            cf.dropPuyoList(cpl);

            callback(std::move(cf), cpl);
        };

        // Finds 2-multi or 3-multi.
        detect(detectedField, strategy, PurposeForFindingRensa::FOR_KEY, prohibitedColumns, detectCallback);
    }
}

void RensaDetector::complementKeyPuyosOn13thRow(const CoreField& originalField,
                                                const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                                const ComplementCallback& callback)
{
    CoreField cf(originalField);
    ColumnPuyoList cpl;
    complementKeyPuyos13thRowInternal(cf, cpl, allowsComplements, 1, callback);
}

void RensaDetector::complementKeyPuyos13thRowInternal(CoreField& currentField,
                                                      ColumnPuyoList& currentKeyPuyos,
                                                      const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                                      int x,
                                                      const ComplementCallback& callback)
{
    if (x > 6) {
        callback(CoreField(currentField), const_cast<const ColumnPuyoList&>(currentKeyPuyos));
        return;
    }

    if (currentField.height(x) == 12 && allowsComplements[x]) {
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            if (!currentField.dropPuyoOn(x, c))
                continue;
            if (!currentKeyPuyos.add(x, c)) {
                currentField.removePuyoFrom(x);
                continue;
            }

            complementKeyPuyos13thRowInternal(currentField, currentKeyPuyos, allowsComplements, x + 1, callback);

            currentField.removePuyoFrom(x);
            currentKeyPuyos.removeTopFrom(x);
        }
    }

    complementKeyPuyos13thRowInternal(currentField, currentKeyPuyos, allowsComplements, x + 1, callback);
}

// static
void RensaDetector::makeProhibitArray(const CoreField& originalField,
                                      const RensaDetectorStrategy& strategy,
                                      const RensaLastVanishedPositionTrackResult& trackResult,
                                      const ColumnPuyoList& firePuyos,
                                      bool prohibits[FieldConstant::MAP_WIDTH])
{
    if (strategy.mode() == RensaDetectorStrategy::Mode::EXTEND) {
        std::fill(prohibits, prohibits + FieldConstant::MAP_WIDTH, false);
        for (int x = 1; x <= 6; ++x)
            prohibits[x] = (firePuyos.sizeOn(x) > 0);
        return;
    }

    std::fill(prohibits, prohibits + FieldConstant::MAP_WIDTH, true);

    trackResult.lastVanishedPositionBits().iterateBitPositions([&originalField, &prohibits](int x, int y) {
        if (originalField.isEmpty(x, y + 1)) {
            prohibits[x] = false;
        } else {
            prohibits[x - 1] = false;
            prohibits[x] = false;
            prohibits[x + 1] = false;
        }
    });

    for (int x = 1; x <= 6; ++x) {
        if (firePuyos.sizeOn(x) > 0)
            prohibits[x] = true;
    }
}
