#ifndef TEST_LOCKIT_CPU_RUN_H_
#define TEST_LOCKIT_CPU_RUN_H_

#include "core/client/ai/raw_ai.h"

#include "coma.h"
#include "cpu_configuration.h"
#include "read.h"

namespace test_lockit {

class TestLockitAI : public RawAI {
public:
    explicit TestLockitAI(const Configuration& configuration);

protected:
    FrameResponse playOneFrame(const FrameRequest&) override;

private:
    const Configuration config;
    COMAI_HI coma;
    COMAI_HI coma2x;
    READ_P r_player[2];
};

}  // namespace test_lockit

#endif
