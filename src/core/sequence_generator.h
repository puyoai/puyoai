#ifndef CORE_SEQUENCE_GENERATOR_H_
#define CORE_SEQUENCE_GENERATOR_H_

#include <random>

#include "core/kumipuyo_seq.h"

KumipuyoSeq generateSequence();

KumipuyoSeq generateRandomSequence();
KumipuyoSeq generateRandomSequenceWithSeed(int seed);
KumipuyoSeq generateRandomSequenceWithMt19937(std::mt19937);

#endif
