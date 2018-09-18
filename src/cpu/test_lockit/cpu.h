#ifndef TEST_LOCKIT_CPU_RUN_H_
#define TEST_LOCKIT_CPU_RUN_H_

#include <fstream>
#include <string>

#include "core/client/ai/raw_ai.h"

#include "coma.h"
#include "cpu_configuration.h"
#include "read.h"

namespace test_lockit {

class TestLockitAI : public RawAI {
public:
    explicit TestLockitAI(const cpu::Configuration& configuration);

    // Run a end-to-end using the file.
    void runTest(const std::string& filename);

protected:
    FrameResponse playOneFrame(const FrameRequest&) override;

private:
    const cpu::Configuration config;
    COMAI_HI coma;
    COMAI_HI coma2x;
    READ_P r_player[2];
    std::ofstream play_log_;
    std::string last_log_;
};

}  // namespace test_lockit

#endif
