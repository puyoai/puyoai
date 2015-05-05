#include "pattern_rensa_detector.h"

#include <algorithm>

#include "base/slice.h"

using namespace std;

namespace {
const int MAX_UNUSED_VARIABLES = 1;
}

void PatternRensaDetector::iteratePossibleRensas(const vector<int>& matchableIds,
                                                 int maxIteration)
{
    DCHECK_GE(maxIteration, 1);

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    // --- Iterate with complementing
    for (const int id : matchableIds) {
        const PatternBookField& pbf = patternBook_.patternBookField(id);
        int x = pbf.ignitionColumn();
        if (x == 0)
            continue;

        double patternScore = 0.0;
        auto scoreCallback = [this, &patternScore, &pbf](int x, int y, double score) {
            if (isNormalColor(originalField_.color(x, y)))
                patternScore += score / pbf.numVariables();
        };

        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(originalField_, MAX_UNUSED_VARIABLES, &cpl, scoreCallback);
        if (!complementResult.success)
            continue;
        if (complementResult.numFilledUnusedVariables > 0)
            continue;

        // No ignition puyo.
        if (cpl.sizeOn(x) == 0)
            continue;

        CoreField cf(originalField_);
        if (!cf.dropPuyoListWithMaxHeight(cpl, maxHeight))
            continue;

        ColumnPuyo firePuyo(x, cpl.get(x, cpl.sizeOn(x) - 1));
        ColumnPuyoList keyPuyos(cpl);
        keyPuyos.removeTopFrom(x);

        CoreField::SimulationContext context(originalContext_);
        RensaYPositionTracker tracker;

        RensaStepResult stepResult = cf.vanishDrop(&context, &tracker);
        if (stepResult.score == 0) {
            CoreField tmp(originalField_);
            tmp.dropPuyoListWithMaxHeight(cpl, maxHeight);
            CHECK_GT(stepResult.score, 0) << tmp.toDebugString();
        }
        int restUnusedVariables = MAX_UNUSED_VARIABLES - complementResult.numFilledUnusedVariables;
        iteratePossibleRensasInternal(cf, context, tracker, 1, firePuyo, keyPuyos,
                                      maxIteration - 1, restUnusedVariables,
                                      pbf.name(), patternScore);
    }

    // --- Iterate without complementing.
    auto detectCallback = [&](const CoreField& complementedField, const ColumnPuyoList& cpl) {
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
        if (!checkDup(firePuyo, keyPuyos))
            return;

        RensaYPositionTracker tracker;
        iteratePossibleRensasInternal(complementedField, originalContext_, tracker, 0,
                                      firePuyo, keyPuyos, maxIteration - 1, 0, string(), 0.0, false);
    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detect(originalField_, strategy_, PurposeForFindingRensa::FOR_FIRE, prohibits, detectCallback);
}

void PatternRensaDetector::iteratePossibleRensasInternal(const CoreField& currentField,
                                                         const CoreField::SimulationContext& currentFieldContext,
                                                         const RensaYPositionTracker& currentFieldTracker,
                                                         int currentChains,
                                                         const ColumnPuyo& firePuyo,
                                                         const ColumnPuyoList& originalKeyPuyos,
                                                         int restIteration,
                                                         int restUnusedVariables,
                                                         const std::string& patternName,
                                                         double currentPatternScore,
                                                         bool addsPatternScore)
{
    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    // With complement.
    // TODO(mayah): making std::vector is too slow. call currentField.fillErasingPuyoPosition()?
    Position ignitionPositions[FieldConstant::WIDTH * FieldConstant::HEIGHT];
    int size = currentField.fillErasingPuyoPositions(currentFieldContext, ignitionPositions);

    // because of PuyoColor::IRON, sometimes we might have valid erasing puyo.
    if (size < 4)
        return;

    std::sort(ignitionPositions, ignitionPositions + size);
    Slice<Position> slice(ignitionPositions, size);
    std::pair<PatternBook::IndexIterator, PatternBook::IndexIterator> p = patternBook_.find(slice);
    bool needsToProceedWithoutComplement = true;
    for (PatternBook::IndexIterator it = p.first; it != p.second; ++it) {
        const PatternBookField& pbf = patternBook_.patternBookField(*it);

        double patternScore = currentPatternScore;
        auto addScoreCallback = [this, &patternScore, &pbf, &currentFieldTracker](int x, int y, double score) {
            int actualY = currentFieldTracker.originalY(x, y);
            if (isNormalColor(originalField_.color(x, actualY)))
                patternScore += score / pbf.numVariables();
        };
        auto dontAddScoreCallback = [](int, int, double) {};

        ColumnPuyoList cpl;
        ComplementResult complementResult = addsPatternScore ?
            pbf.complement(currentField, restUnusedVariables, &cpl, addScoreCallback) :
            pbf.complement(currentField, restUnusedVariables, &cpl, dontAddScoreCallback);

        if (!complementResult.success)
            continue;

        if (cpl.size() == 0) {
            needsToProceedWithoutComplement = false;

            CoreField cf(currentField);
            CoreField::SimulationContext context(currentFieldContext);
            RensaYPositionTracker tracker(currentFieldTracker);
            RensaStepResult stepResult = cf.vanishDrop(&context, &tracker);
            CHECK_GT(stepResult.score, 0);

            iteratePossibleRensasInternal(cf, context, tracker, currentChains + 1, firePuyo, originalKeyPuyos,
                                          restIteration, restUnusedVariables,
                                          patternName.empty() ? pbf.name() : patternName,
                                          patternScore);
            continue;
        }

        if (restIteration <= 0)
            continue;

        if (cpl.sizeOn(firePuyo.x) > 0)
            continue;

        ColumnPuyoList keyPuyos(originalKeyPuyos);
        if (!keyPuyos.merge(cpl))
            continue;
        if (!checkDup(firePuyo, keyPuyos))
            continue;

        CoreField cf(currentField);
        if (!cf.dropPuyoListWithMaxHeight(cpl, maxHeight))
            continue;

        CoreField::SimulationContext context(currentFieldContext);
        RensaYPositionTracker tracker(currentFieldTracker);
        RensaStepResult stepResult = cf.vanishDrop(&context, &tracker);
        CHECK_GT(stepResult.score, 0);

        iteratePossibleRensasInternal(cf, context, tracker, currentChains + 1, firePuyo, keyPuyos,
                                      restIteration - 1,
                                      restUnusedVariables - complementResult.numFilledUnusedVariables,
                                      patternName.empty() ? pbf.name() : patternName,
                                      patternScore);
    }

    if (!needsToProceedWithoutComplement)
        return;

    // proceed one without complementing.
    CoreField cf(currentField);
    CoreField::SimulationContext context(currentFieldContext);
    RensaYPositionTracker tracker(currentFieldTracker);
    CHECK(cf.vanishDrop(&context, &tracker).score > 0) << cf.toDebugString();

    // If rensa continues, proceed to next.
    if (cf.rensaWillOccurWithContext(context)) {
        iteratePossibleRensasInternal(cf, context, tracker, currentChains + 1, firePuyo, originalKeyPuyos,
                                      restIteration, restUnusedVariables, patternName, currentPatternScore);
        return;
    }

    // if currentField does not erase anything...
    bool prohibits[FieldConstant::MAP_WIDTH] {};
    if (!checkRensa(currentChains + 1, firePuyo, originalKeyPuyos, currentPatternScore, patternName, prohibits))
        return;
    if (restIteration <= 0)
        return;
    auto detectCallback = [&](const CoreField& cf2, const ColumnPuyoList& cpl) {
        if (cpl.size() == 0)
            return;

        ColumnPuyoList keyPuyos(originalKeyPuyos);
        if (!keyPuyos.merge(cpl))
            return;
        if (!checkDup(firePuyo, keyPuyos))
            return;

        iteratePossibleRensasInternal(cf2, context, tracker, currentChains + 1,
                                      firePuyo, keyPuyos, restIteration - 1, restUnusedVariables,
                                      patternName, currentPatternScore);
    };
    RensaDetector::detect(cf, strategy_, PurposeForFindingRensa::FOR_KEY, prohibits, detectCallback);
}

bool PatternRensaDetector::checkDup(const ColumnPuyo& firePuyo,
                                    const ColumnPuyoList& keyPuyos)
{
    ColumnPuyoList whole(keyPuyos);
    whole.add(firePuyo);
    if (usedSet_.count(whole))
        return false;
    usedSet_.insert(whole);
    return true;
}

bool PatternRensaDetector::checkRensa(int currentChains,
                                      const ColumnPuyo& firePuyo,
                                      const ColumnPuyoList& keyPuyos,
                                      double patternScore,
                                      const std::string& patternName,
                                      bool prohibits[FieldConstant::MAP_WIDTH])
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

    RensaChainTracker tracker;
    RensaResult rensaResult = cf.simulate(&context, &tracker);
    if (rensaResult.chains != currentChains)
        return false;

    ColumnPuyoList puyosToComplement(keyPuyos);
    if (!puyosToComplement.add(firePuyo))
        return false;

    callback_(cf, rensaResult, puyosToComplement, firePuyo.color, tracker.result(), patternName, patternScore);

    // TODO(mayah): Making ColumnPuyoList here is time-consuming a bit.
    ColumnPuyoList firePuyos;
    firePuyos.add(firePuyo);
    RensaDetector::makeProhibitArray(rensaResult, tracker.result(), originalField_, firePuyos, prohibits);

    return true;
}
