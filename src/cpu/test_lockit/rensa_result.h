#ifndef CPU_TEST_LOCKIT_RENSA_RESULT_H_
#define CPU_TEST_LOCKIT_RENSA_RESULT_H_

namespace test_lockit {

struct TLRensaResult {
    static const int MAX_RENSA = 19;

    int chains = 0;

    // total score in a continuous chains
    int score = 0;

    // total colored puyos vanished in a continuous chains
    int num_vanished = 0;

    // true if the last vanishment drops no puyos
    bool quick = false;

    // the number of vanishing groups in each chain step.  The values of
    // num_connections[i] for i >= chains are not defined
    int num_connections[MAX_RENSA] {};
};

} // namespace test_lockit

#endif // CPU_TEST_LOCKIT_RENSA_RESULT_H_
