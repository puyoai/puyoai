#include "pattern_rensa_detector.h"

#include <algorithm>

#include <gflags/gflags.h>

using namespace std;

DEFINE_bool(use_side_chain, false, "Use sidechain iteration");

namespace {
const int MAX_UNUSED_VARIABLES_FOR_FIRST_PATTERN = 0;
const int MAX_UNUSED_VARIABLES = 1;
}

void PatternRensaDetector::iteratePossibleRensas(const vector<int>& /*matchableIds*/,
                                                 int maxIteration)
{
    DCHECK_GE(maxIteration, 1);

    const int maxHeight = strategy_.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    // --- Iterate with complementing pattern.
    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl,
                        int numFilledUnusedVariables, const FieldBits& matchedBits,
                        const NewPatternBookField& pbf) {
        int x = pbf.ignitionColumn();
        if (x == 0)
            return;

        // We don't allow unused variable for the first pattern.
        if (numFilledUnusedVariables > 0)
            return;

        // No ignition puyo.
        if (cpl.sizeOn(x) == 0)
            return;

        ColumnPuyo firePuyo(x, cpl.get(x, cpl.sizeOn(x) - 1));
        ColumnPuyoList keyPuyos(cpl);
        keyPuyos.removeTopFrom(x);
        if (!checkDup(firePuyo, keyPuyos))
            return;

        RensaExistingPositionTracker tracker(originalField_.bitField().normalColorBits());
        if (!complementedField.vanishDropFast(&tracker)) {
            CoreField tmp(originalField_);
            tmp.dropPuyoListWithMaxHeight(cpl, maxHeight);

            // TODO(mayah): This shouldn't happen. However, this sometimes triggered now.
            LOG(ERROR) << tmp.toDebugString();
            return;
        }

        // For here, we don't need to make AND to RensaExistingPositionTracker.
        double patternScore = pbf.score() * matchedBits.popcount() / pbf.numVariables();
        int restUnusedVariables = MAX_UNUSED_VARIABLES - numFilledUnusedVariables;
        iteratePossibleRensasInternal(complementedField, tracker, 1, firePuyo, keyPuyos,
                                      maxIteration - 1, restUnusedVariables,
                                      pbf.name(), patternScore);
    };
    patternBook_.complement(originalField_, MAX_UNUSED_VARIABLES_FOR_FIRST_PATTERN, callback);

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
    // With complement.
    // TODO(mayah): making std::vector is too slow. call currentField.fillErasingPuyoPosition()?
    FieldBits ignitionPosition = currentField.ignitionPuyoBits();

    // because of PuyoColor::IRON, sometimes we might have valid erasing puyo.
    if (ignitionPosition.isEmpty())
        return;

    bool needsToProceedWithoutComplement = true;
    auto callback = [&](CoreField&& complementedField, const ColumnPuyoList& cpl,
                        int numFilledUnusedVariables, const FieldBits& matchedBits,
                        const NewPatternBookField& pbf) {
        double patternScore = currentPatternScore;
        if (addsPatternScore) {
            FieldBits currentMatchedBits = matchedBits & currentFieldTracker.result().existingBits();
            patternScore += pbf.score() * currentMatchedBits.popcount() / pbf.numVariables();
        }

        if (cpl.size() == 0) {
            needsToProceedWithoutComplement = false;

            RensaExistingPositionTracker tracker(currentFieldTracker);
            CHECK(complementedField.vanishDropFast(&tracker));

            iteratePossibleRensasInternal(complementedField, tracker, currentChains + 1, firePuyo, originalKeyPuyos,
                                          restIteration, restUnusedVariables,
                                          patternName.empty() ? pbf.name() : patternName,
                                          patternScore);
            return;
        }

        if (restIteration <= 0)
            return;

        if (cpl.sizeOn(firePuyo.x) > 0)
            return;

        ColumnPuyoList keyPuyos(originalKeyPuyos);
        if (!keyPuyos.merge(cpl))
            return;
        if (!checkDup(firePuyo, keyPuyos))
            return;

        RensaExistingPositionTracker tracker(currentFieldTracker);
        if (!complementedField.vanishDropFast(&tracker)) {
            LOG(ERROR) << complementedField.toDebugString();
            return;
        }

        iteratePossibleRensasInternal(complementedField, tracker, currentChains + 1, firePuyo, keyPuyos,
                                      restIteration - 1,
                                      restUnusedVariables - numFilledUnusedVariables,
                                      patternName.empty() ? pbf.name() : patternName,
                                      patternScore);
    };
    patternBook_.complement(currentField, ignitionPosition, restUnusedVariables, callback);

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
