#include "pattern_rensa_detector.h"

#include <algorithm>

#include <gflags/gflags.h>

using namespace std;

DEFINE_bool(use_side_chain, false, "Use sidechain iteration");

namespace {
const int MAX_UNUSED_VARIABLES_FOR_FIRST_PATTERN = 0;
const int MAX_UNUSED_VARIABLES = 1;
}

void PatternRensaDetector::iteratePossibleRensas(const vector<int>& matchableIds,
                                                 int maxIteration)
{
    DCHECK_GE(maxIteration, 1);

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    // --- Iterate with complementing pattern.
    for (const int id : matchableIds) {
        const PatternBookField& pbf = patternBook_.patternBookField(id);
        int x = pbf.ignitionColumn();
        if (x == 0)
            continue;

        ComplementResult complementResult = pbf.complement(originalField_, MAX_UNUSED_VARIABLES_FOR_FIRST_PATTERN);

        if (!complementResult.success)
            continue;
        // We don't allow unused variable for the first pattern.
        if (complementResult.numFilledUnusedVariables > 0)
            continue;

        const ColumnPuyoList& cpl = complementResult.complementedPuyoList;

        // No ignition puyo.
        if (complementResult.complementedPuyoList.sizeOn(x) == 0)
            continue;

        CoreField cf(originalField_);
        if (!cf.dropPuyoListWithMaxHeight(cpl, maxHeight))
            continue;

        ColumnPuyo firePuyo(x, cpl.get(x, cpl.sizeOn(x) - 1));
        ColumnPuyoList keyPuyos(cpl);
        keyPuyos.removeTopFrom(x);
        if (!checkDup(firePuyo, keyPuyos))
            continue;

        RensaExistingPositionTracker tracker(originalField_.bitField().normalColorBits());
        if (!cf.vanishDropFast(&tracker)) {
            CoreField tmp(originalField_);
            tmp.dropPuyoListWithMaxHeight(cpl, maxHeight);

            // TODO(mayah): This shouldn't happen. However, this sometimes triggered now.
            LOG(ERROR) << tmp.toDebugString();
            continue;
        }

        // For here, we don't need to make AND to RensaExistingPositionTracker.
        FieldBits matchedBits = complementResult.matchedResult.matchedBits;
        double patternScore = pbf.score() * matchedBits.popcount() / pbf.numVariables();
        int restUnusedVariables = MAX_UNUSED_VARIABLES - complementResult.numFilledUnusedVariables;
        iteratePossibleRensasInternal(cf, tracker, 1, firePuyo, keyPuyos,
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

        RensaExistingPositionTracker tracker(originalField_.bitField().normalColorBits());
        iteratePossibleRensasInternal(complementedField, tracker, 0,
                                      firePuyo, keyPuyos, maxIteration - 1, 0, string(), 0.0, false);

        if (FLAGS_use_side_chain) {
            auto sideChainCallback = [&](CoreField&& cf, const ColumnPuyoList& puyosToComplement) {
                const RensaResult rensaResult = cf.simulate();
                // TODO(mayah): firePuyo should be accurate.
                const PuyoColor firePuyoColor = PuyoColor::RED;
                callback_(cf, rensaResult, puyosToComplement, firePuyoColor, "", 0.0);
            };
            RensaDetector::detectSideChainFromDetectedField(
                originalField_, complementedField, strategy_, cpl, sideChainCallback);
        }
    };

    const bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detect(originalField_, strategy_, PurposeForFindingRensa::FOR_FIRE, prohibits, detectCallback);
}

void PatternRensaDetector::iteratePossibleRensasInternal(const CoreField& currentField,
                                                         const RensaExistingPositionTracker& currentFieldTracker,
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
    FieldBits ignitionPosition = currentField.ignitionPuyoBits();

    // because of PuyoColor::IRON, sometimes we might have valid erasing puyo.
    if (ignitionPosition.isEmpty())
        return;

    std::pair<PatternBook::IndexIterator, PatternBook::IndexIterator> p = patternBook_.find(ignitionPosition);
    bool needsToProceedWithoutComplement = true;
    for (PatternBook::IndexIterator it = p.first; it != p.second; ++it) {
        const PatternBookField& pbf = patternBook_.patternBookField(*it);

        ComplementResult complementResult = pbf.complement(currentField, restUnusedVariables);
        if (!complementResult.success)
            continue;

        double patternScore = currentPatternScore;
        if (addsPatternScore) {
            FieldBits matchedBits = complementResult.matchedResult.matchedBits & currentFieldTracker.result().existingBits();
            patternScore += pbf.score() * matchedBits.popcount() / pbf.numVariables();
        }

        const ColumnPuyoList& cpl = complementResult.complementedPuyoList;
        if (cpl.size() == 0) {
            needsToProceedWithoutComplement = false;

            CoreField cf(currentField);
            RensaExistingPositionTracker tracker(currentFieldTracker);
            CHECK(cf.vanishDropFast(&tracker));

            iteratePossibleRensasInternal(cf, tracker, currentChains + 1, firePuyo, originalKeyPuyos,
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

        RensaExistingPositionTracker tracker(currentFieldTracker);
        if (!cf.vanishDropFast(&tracker)) {
            LOG(ERROR) << cf.toDebugString();
            continue;
        }

        iteratePossibleRensasInternal(cf, tracker, currentChains + 1, firePuyo, keyPuyos,
                                      restIteration - 1,
                                      restUnusedVariables - complementResult.numFilledUnusedVariables,
                                      patternName.empty() ? pbf.name() : patternName,
                                      patternScore);
    }

    if (!needsToProceedWithoutComplement)
        return;

    // proceed one without complementing.
    CoreField cf(currentField);
    RensaExistingPositionTracker tracker(currentFieldTracker);
    CHECK(cf.vanishDropFast(&tracker)) << cf.toDebugString();

    // If rensa continues, proceed to next.
    if (cf.rensaWillOccur()) {
        iteratePossibleRensasInternal(cf, tracker, currentChains + 1, firePuyo, originalKeyPuyos,
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

        iteratePossibleRensasInternal(cf2, tracker, currentChains + 1,
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

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;
    if (!cf.dropPuyoListWithMaxHeight(keyPuyos, maxHeight))
        return false;

    // If rensa occurs after adding key puyos, this is invalid.
    if (cf.rensaWillOccur())
        return false;

    if (!cf.dropPuyoOn(firePuyo.x, firePuyo.color))
        return false;

    RensaLastVanishedPositionTracker tracker;
    RensaResult rensaResult = cf.simulate(&tracker);
    if (rensaResult.chains != currentChains)
        return false;

    ColumnPuyoList puyosToComplement(keyPuyos);
    if (!puyosToComplement.add(firePuyo))
        return false;

    callback_(cf, rensaResult, puyosToComplement, firePuyo.color, patternName, patternScore);

    // TODO(mayah): Making ColumnPuyoList here is time-consuming a bit.
    ColumnPuyoList firePuyos;
    firePuyos.add(firePuyo);
    RensaDetector::makeProhibitArray(originalField_, strategy_, tracker.result(), firePuyos, prohibits);

    return true;
}
