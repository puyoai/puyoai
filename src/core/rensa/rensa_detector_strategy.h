#ifndef CORE_RENSA_RENSA_DETECTOR_STRATEGY_H_
#define CORE_RENSA_RENSA_DETECTOR_STRATEGY_H_

class RensaDetectorStrategy {
public:
    enum class Mode {
        DROP, FLOAT, EXTEND,
    };

    RensaDetectorStrategy(Mode mode,
                          int maxNumOfComplementPuyosForKey,
                          int maxNumOfComplementPuyosForFire,
                          bool allowsPuttingKeyPuyoOn13thRow) :
        mode_(mode),
        maxNumOfComplementPuyosForKey_(maxNumOfComplementPuyosForKey),
        maxNumOfComplementPuyosForFire_(maxNumOfComplementPuyosForFire),
        allowsPuttingKeyPuyoOn13thRow_(allowsPuttingKeyPuyoOn13thRow)
    {
    }

    static RensaDetectorStrategy defaultFloatStrategy() { return RensaDetectorStrategy(Mode::FLOAT, 3, 3, true); }
    static RensaDetectorStrategy defaultDropStrategy() { return RensaDetectorStrategy(Mode::DROP, 3, 3, true); }
    static RensaDetectorStrategy defaultExtendStrategy() { return RensaDetectorStrategy(Mode::EXTEND, 3, 3, false); }

    Mode mode() const { return mode_; }
    int maxNumOfComplementPuyosForKey() const { return maxNumOfComplementPuyosForKey_; }
    int maxNumOfComplementPuyosForFire() const { return maxNumOfComplementPuyosForFire_; }
    bool allowsPuttingKeyPuyoOn13thRow() const { return allowsPuttingKeyPuyoOn13thRow_; }
    int maxKeyPuyoHeight() const { return allowsPuttingKeyPuyoOn13thRow() ? 13 : 12; }

private:
    Mode mode_;
    int maxNumOfComplementPuyosForKey_;
    int maxNumOfComplementPuyosForFire_;
    bool allowsPuttingKeyPuyoOn13thRow_;
};

#endif // CORE_RENSA_RENSA_DETECTOR_STRATEGY_H_
