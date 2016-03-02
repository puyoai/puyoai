#ifndef TEST_LOCKIT_CPU_RUN_H_
#define TEST_LOCKIT_CPU_RUN_H_

#include "core/client/ai/raw_ai.h"

#include "coma.h"
#include "cpu_configuration.h"
#include "read.h"

namespace test_lockit {

class TestLockitAI : public RawAI {
public:
    explicit TestLockitAI(const cpu::Configuration& configuration);

protected:
    FrameResponse playOneFrame(const FrameRequest&) override;

    void onGameWillBegin(const FrameRequest&);
    void onGameHasEnded(const FrameRequest&);

    void onPreDecisionRequestedForMe(const FrameRequest&);
    void onDecisionRequestedForMe(const FrameRequest&);
    void onGroundedForMe(const FrameRequest&);
    void onPuyoErasedForMe(const FrameRequest&);
    void onOjamaDroppedForMe(const FrameRequest&);
    void onNext2AppearedForMe(const FrameRequest&);

    void onDecisionRequestedForEnemy(const FrameRequest&);
    void onGroundedForEnemy(const FrameRequest&);
    void onPuyoErasedForEnemy(const FrameRequest&);
    void onOjamaDroppedForEnemy(const FrameRequest&);
    void onNext2AppearedForEnemy(const FrameRequest&);

private:
    const cpu::Configuration config;
    COMAI_HI coma;
    COMAI_HI coma2x;
    READ_P r_player[2];
};

}  // namespace test_lockit

#endif
