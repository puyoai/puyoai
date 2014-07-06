#ifndef CORE_SEQUENCE_GENERATOR_H_
#define CORE_SEQUENCE_GENERATOR_H_

#include <random>

#include "core/kumipuyo.h"

KumipuyoSeq generateSequence();
KumipuyoSeq generateSequenceWithSeed(int seed);
KumipuyoSeq generateSequenceWithMt19937(std::mt19937);

#endif
