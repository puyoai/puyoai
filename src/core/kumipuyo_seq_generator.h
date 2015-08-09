#ifndef CORE_KUMIPUYO_SEQ_GENERATOR_H_
#define CORE_KUMIPUYO_SEQ_GENERATOR_H_

#include <random>

#include "base/noncopyable.h"
#include "core/kumipuyo_seq.h"

class KumipuyoSeqGenerator : noncopyable {
public:
    // Generates random kumipuyo sequence. It's completely random.
    static KumipuyoSeq generateRandomSequence(int size);
    static KumipuyoSeq generateRandomSequenceWithSeed(int size, int seed);
    static KumipuyoSeq generateRandomSequenceWithMt19937(int size, std::mt19937*);

    // Generates AC puyo2 random sequence.
    // This can be configured via flags.
    static KumipuyoSeq generateACPuyo2Sequence();
    static KumipuyoSeq generateACPuyo2SequenceWithSeed(int seed);
    static KumipuyoSeq generateACPuyo2SequenceWithMt19937(std::mt19937*);

private:
    KumipuyoSeqGenerator() = delete;
    ~KumipuyoSeqGenerator() = delete;
};

#endif // CORE_KUMIPUYO_SEQ_GENERATOR_H_
