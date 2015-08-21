#ifndef CORE_RENSA_TRACKER_RENSA_COEF_TRACKER_H_
#define CORE_RENSA_TRACKER_RENSA_COEF_TRACKER_H_

#include "core/score.h"
#include "core/rensa_tracker.h"

// RensaCoefResult represents
//  - the number of erased puyo in each chain
//  - the coefficient in each chain.
class RensaCoefResult {
public:
    RensaCoefResult() : numErased_{}, longBonusCoef_{}, colorBonusCoef_{}
    {
    }

    void setCoef(int nth, int numErased, int longBonusCoef, int colorBonusCoef)
    {
        numErased_[nth] = numErased;
        longBonusCoef_[nth] = longBonusCoef;
        colorBonusCoef_[nth] = colorBonusCoef;
    }

    int numErased(int nth) const { return numErased_[nth]; }
    int longBonusCoef(int nth) const { return longBonusCoef_[nth]; }
    int colorBonusCoef(int nth) const { return colorBonusCoef_[nth]; }

    int coef(int nth) const
    {
        return calculateRensaBonusCoef(chainBonus(nth), this->longBonusCoef(nth), this->colorBonusCoef(nth));
    }

    int score(int additionalChain) const;

private:
    int numErased_[20]; // numErased does not contain ojama puyos.
    int longBonusCoef_[20];
    int colorBonusCoef_[20];
};

template<>
class RensaTracker<RensaCoefResult> {
public:
    const RensaCoefResult& result() const { return result_; }

    void trackCoef(int nthChain, int numErasedPuyo, int longBonusCoef, int colorBonusCoef)
    {
        result_.setCoef(nthChain, numErasedPuyo, longBonusCoef, colorBonusCoef);
    }

    void trackVanish(int /*nthChain*/, const FieldBits& /*vanishedColorPuyoBits*/, const FieldBits& /*vanishedOjamaPuyoBits*/) {}
    void trackDrop(FieldBits /*blender*/, FieldBits /*leftOnes*/, FieldBits /*rightOnes*/) {}
#ifdef __BMI2__
    void trackDropBMI2(std::uint64_t /*oldLowBits*/, std::uint64_t /*oldHighBits*/, std::uint64_t /*newLowBits*/, std::uint64_t /*newHighBits*/) {}
#endif

private:
    RensaCoefResult result_;
};
typedef RensaTracker<RensaCoefResult> RensaCoefTracker;

#endif // CORE_RENSA_TRACKER_RENSA_COEF_TRACKER_H_
