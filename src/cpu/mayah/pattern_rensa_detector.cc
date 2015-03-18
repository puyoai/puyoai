#include "pattern_rensa_detector.h"

#include <algorithm>

using namespace std;

namespace {
const int MAX_UNUSED_VARIABLES = 1;
}

void PatternRensaDetector::iteratePossibleRensas(const vector<int>& matchableIds,
                                                 int maxIteration) const
{
    DCHECK_GE(maxIteration, 1);

    // --- Iterate with complementing
    for (const int id : matchableIds) {
        const PatternBookField& pbf = patternBook_.patternBookField(id);
        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(originalField_, MAX_UNUSED_VARIABLES, &cpl);
        if (!complementResult.success)
            continue;
        if (complementResult.numFilledUnusedVariables > 0)
            continue;

        CoreField cf(originalField_);
        bool ok = true;
        bool foundFirePuyo = false;
        ColumnPuyo firePuyo;
        ColumnPuyoList keyPuyos;
        for (const ColumnPuyo& cp : cpl) {
            if (!cf.dropPuyoOn(cp.x, cp.color)) {
                ok = false;
                break;
            }
            if (foundFirePuyo) {
                if (cp.x == pbf.ignitionColumn()) {
                    keyPuyos.add(firePuyo);
                    firePuyo = cp;
                } else if (!keyPuyos.add(cp)) {
                    ok = false;
                    break;
                }
            } else {
                if (cp.x == pbf.ignitionColumn()) {
                    foundFirePuyo = true;
                    firePuyo = cp;
                } else {
                    keyPuyos.add(cp);
                }
            }
        }

        if (!ok || !foundFirePuyo)
            continue;

        CoreField::SimulationContext context(originalContext_);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        int restUnusedVariables = MAX_UNUSED_VARIABLES - complementResult.numFilledUnusedVariables;
        double ratio = 0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            for (const auto& cp : cpl) {
                if (isNormalColor(cp.color))
                    ++count;
            }
            double d = pbf.numVariables() - count;
            ratio = d / pbf.numVariables();
        }
        iteratePossibleRensasInternal(cf, context, 1, firePuyo, keyPuyos,
                                      maxIteration - 1, restUnusedVariables,
                                      pbf.name(), pbf.score() * ratio);
    }

    // --- Iterate without complementing.
    auto detectCallback = [&](CoreField* cf, const ColumnPuyoList& cpl) {
        if (cpl.size() == 0)
            return;

        bool first = true;
        ColumnPuyo firePuyo;
        ColumnPuyoList keyPuyos;
        for (const ColumnPuyo& cp : cpl) {
            if (first) {
                firePuyo = cp;
                first = false;
                continue;
            }

            keyPuyos.add(firePuyo);
            firePuyo = cp;
        }

        iteratePossibleRensasInternal(*cf, originalContext_, 0,
                                      firePuyo, keyPuyos, maxIteration - 1, 0, string(), 0);

    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detect(originalField_, strategy_, PurposeForFindingRensa::FOR_FIRE, prohibits, detectCallback);
}

void PatternRensaDetector::iteratePossibleRensasInternal(const CoreField& currentField,
                                                         const CoreField::SimulationContext& currentFieldContext,
                                                         int currentChains,
                                                         const ColumnPuyo& firePuyo,
                                                         const ColumnPuyoList& originalKeyPuyos,
                                                         int restIteration,
                                                         int restUnusedVariables,
                                                         const std::string& patternName,
                                                         double patternScore) const
{
    // With complement.
    // TODO(mayah): making std::vector is too slow. call currentField.fillErasingPuyoPosition()?
    std::vector<Position> ignitionPositions = currentField.erasingPuyoPositions(currentFieldContext);
    DCHECK(!ignitionPositions.empty()) << currentField.toDebugString();
    if (ignitionPositions.empty())
        return;

    std::sort(ignitionPositions.begin(), ignitionPositions.end());
    std::pair<PatternBook::IndexIterator, PatternBook::IndexIterator> p = patternBook_.find(ignitionPositions);
    bool needsToProceedWithoutComplement = true;
    for (PatternBook::IndexIterator it = p.first; it != p.second; ++it) {
        const PatternBookField& pbf = patternBook_.patternBookField(it->second);

        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(currentField, restUnusedVariables, &cpl);
        if (!complementResult.success)
            continue;

        if (cpl.size() == 0) {
            needsToProceedWithoutComplement = false;

            CoreField cf(currentField);
            CoreField::SimulationContext context(currentFieldContext);
            int score = cf.vanishDrop(&context);
            CHECK(score > 0) << score;

            iteratePossibleRensasInternal(cf, context, currentChains + 1, firePuyo, originalKeyPuyos,
                                          restIteration, restUnusedVariables,
                                          patternName.empty() ? pbf.name() : patternName,
                                          patternScore + pbf.score());
            continue;
        }

        if (restIteration <= 0)
            continue;

        CoreField cf(currentField);
        ColumnPuyoList keyPuyos(originalKeyPuyos);

        bool ok = true;
        for (const ColumnPuyo& cp : cpl) {
            if (cp.x == firePuyo.x) {
                ok = false;
                break;
            }
            if (!keyPuyos.add(cp)) {
                ok = false;
                break;
            }
            if (!cf.dropPuyoOn(cp.x, cp.color)) {
                ok = false;
                break;
            }
        }

        if (!ok)
            continue;

        CoreField::SimulationContext context(currentFieldContext);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        double ratio = 0.0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            for (const auto& cp : cpl) {
                if (isNormalColor(cp.color))
                    ++count;
            }
            double d = pbf.numVariables() - count;
            ratio = d / pbf.numVariables();
        }

        iteratePossibleRensasInternal(cf, context, currentChains + 1, firePuyo, keyPuyos,
                                      restIteration - 1,
                                      restUnusedVariables - complementResult.numFilledUnusedVariables,
                                      patternName.empty() ? pbf.name() : patternName,
                                      patternScore + pbf.score() * ratio);
    }

    if (!needsToProceedWithoutComplement)
        return;

    // proceed one without complementing.
    CoreField cf(currentField);
    CoreField::SimulationContext context(currentFieldContext);
    CHECK(cf.vanishDrop(&context) > 0) << cf.toDebugString();

    // If rensa continues, proceed to next.
    if (cf.rensaWillOccurWithContext(context)) {
        iteratePossibleRensasInternal(cf, context, currentChains + 1, firePuyo, originalKeyPuyos,
                                      restIteration, restUnusedVariables, patternName, patternScore);
        return;
    }

    // if currentField does not erase anything...
    bool prohibits[FieldConstant::MAP_WIDTH] {};
    if (!checkRensa(currentChains + 1, firePuyo, originalKeyPuyos, patternScore, patternName, prohibits))
        return;
    if (restIteration <= 0)
        return;
    auto detectCallback = [&](CoreField* cf2, const ColumnPuyoList& cpl) {
        if (cpl.size() == 0)
            return;

        bool ok = true;
        ColumnPuyoList keyPuyos(originalKeyPuyos);
        for (const ColumnPuyo& cp : cpl) {
            if (!keyPuyos.add(cp)) {
                ok = false;
                break;
            }
        }

        if (!ok)
            return;

        iteratePossibleRensasInternal(*cf2, context, currentChains + 1,
                                      firePuyo, keyPuyos, restIteration - 1, restUnusedVariables,
                                      patternName, patternScore);
    };
    RensaDetector::detect(cf, strategy_, PurposeForFindingRensa::FOR_KEY, prohibits, detectCallback);
}

bool PatternRensaDetector::checkRensa(int currentChains,
                                      const ColumnPuyo& firePuyo,
                                      const ColumnPuyoList& keyPuyos,
                                      double patternScore,
                                      const std::string& patternName,
                                      bool prohibits[FieldConstant::MAP_WIDTH]) const
{
    CoreField cf(originalField_);
    CoreField::SimulationContext context(originalContext_);

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    for (const auto& cp : keyPuyos) {
        if (!cf.dropPuyoOn(cp.x, cp.color))
            return false;
        if (cf.height(cp.x) > maxHeight)
            return false;
    }

    // If rensa occurs after adding key puyos, this is invalid.
    if (cf.rensaWillOccurWithContext(context))
        return false;

    context.updateFromField(cf);

    if (!cf.dropPuyoOn(firePuyo.x, firePuyo.color))
        return false;

    RensaTrackResult trackResult;
    RensaResult rensaResult = cf.simulateWithContext(&context, &trackResult);
    if (rensaResult.chains != currentChains)
        return false;

    ColumnPuyoList firePuyos;
    if (!firePuyos.add(firePuyo))
        return false;

    callback_(cf, rensaResult, keyPuyos, firePuyos, trackResult, patternName, patternScore);

    RensaDetector::makeProhibitArray(rensaResult, trackResult, originalField_,
                                     firePuyos, prohibits);
    return true;
}
