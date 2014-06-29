#ifndef CLIENT_CONNECTION_DROP_DECISION_H_
#define CLIENT_CONNECTION_DROP_DECISION_H_

#include <stdio.h>
#include <string>

#include "core/decision.h"

class DropDecision {
public:
    explicit DropDecision(const Decision& decision = Decision(),
                          std::string message = std::string()) :
        decision_(decision),
        message_(std::move(message))
    {
    }

    const Decision& decision() const { return decision_; }
    const std::string message() const { return message_; }

private:
    Decision decision_;
    std::string message_;
};

#endif
