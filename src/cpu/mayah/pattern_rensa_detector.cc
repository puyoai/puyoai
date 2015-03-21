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

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    // --- Iterate with complementing
    for (const int id : matchableIds) {
        const PatternBookField& pbf = patternBook_.patternBookField(id);
        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(originalField_, MAX_UNUSED_VARIABLES, &cpl);
        if (!complementResult.success)
            continue;
        if (complementResult.numFilledUnusedVariables > 0)
            continue;

        // No ignition puyo.
        if (cpl.sizeOn(pbf.ignitionColumn()) == 0)
            continue;

        CoreField cf(originalField_);
        if (!cf.dropPuyoListWithMaxHeight(cpl, maxHeight))
            continue;

        int x = pbf.ignitionColumn();
        ColumnPuyo firePuyo(x, cpl.get(x, cpl.sizeOn(x) - 1));
        ColumnPuyoList keyPuyos(cpl);
        keyPuyos.removeTopFrom(x);

        CoreField::SimulationContext context(originalContext_);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        int restUnusedVariables = MAX_UNUSED_VARIABLES - complementResult.numFilledUnusedVariables;
        double ratio = 0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            for (int x = 1; x <= 6; ++x) {
                int h = cpl.sizeOn(x);
                for (int i = 0; i < h; ++i) {
                    if (isNormalColor(cpl.get(x, i)))
                        ++count;
                }
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
        if (cpl.isEmpty())
            return;

        ColumnPuyo firePuyo;
        ColumnPuyoList keyPuyos(cpl);
        for (int x = 1; x <= 6; ++x) {
            if (keyPuyos.sizeOn(x) > 0) {
                firePuyo = ColumnPuyo(x, keyPuyos.get(x, keyPuyos.sizeOn(x) - 1));
                keyPuyos.removeTopFrom(x);
                break;
            }
        }

        DCHECK(firePuyo.isValid());

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
    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

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

        if (cpl.sizeOn(firePuyo.x) > 0)
            continue;

        CoreField cf(currentField);
        if (!cf.dropPuyoListWithMaxHeight(cpl, maxHeight))
            continue;

        ColumnPuyoList keyPuyos(originalKeyPuyos);
        if (!keyPuyos.merge(cpl))
            continue;

        CoreField::SimulationContext context(currentFieldContext);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        double ratio = 0.0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            cpl.iterate([&count](int /*x*/, PuyoColor pc) {
                if (isNormalColor(pc))
                    ++count;
            });
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

        ColumnPuyoList keyPuyos(originalKeyPuyos);
        if (!keyPuyos.merge(cpl))
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
    if (!cf.dropPuyoListWithMaxHeight(keyPuyos, maxHeight))
        return false;

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
