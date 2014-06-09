#ifndef DUEL_USER_INPUT_H_
#define DUEL_USER_INPUT_H_

#include "core/key.h"

class UserInput {
public:
    virtual ~UserInput() {}
    virtual Key getKey() = 0;
};

#endif
