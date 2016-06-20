#ifndef CLIENT_CONNECTION_DROP_DECISION_H_
#define CLIENT_CONNECTION_DROP_DECISION_H_

#include <string>

#include "core/decision.h"

class DropDecision {
public:
    explicit DropDecision(const Decision& decision = Decision(),
                          const std::string& message = std::string()) :
        decision_(decision),
        message_ { message }
    {
    }

    const Decision& decision() const { return decision_; }
    const std::string& message() const { return message_; }

    bool valid() const { return decision_.isValid(); }

private:
    Decision decision_;
    std::string message_;
};

#endif
