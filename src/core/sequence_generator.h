#ifndef CORE_SEQUENCE_GENERATOR_H_
#define CORE_SEQUENCE_GENERATOR_H_

#include <random>

#include "base/base.h"
#include "core/kumipuyo_seq.h"

KumipuyoSeq generateSequence() DEPRECATED_MSG("Use KumipuyoSeqGenerator instead");
KumipuyoSeq generateRandomSequence() DEPRECATED_MSG("Use KumipuyoSeqGenerator instead");
KumipuyoSeq generateRandomSequenceWithSeed(int seed) DEPRECATED_MSG("Use KumipuyoSeqGenerator instead");
KumipuyoSeq generateRandomSequenceWithMt19937(std::mt19937) DEPRECATED_MSG("Use KumipuyoSeqGenerator instead");

#endif
