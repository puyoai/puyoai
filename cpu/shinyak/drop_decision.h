#ifndef __DROP_DECISION_H__
#define __DROP_DECISION_H__

#include <stdio.h>
#include <string>

#include "core/decision.h"

class DropDecision {
public:
    explicit DropDecision(const Decision& decision = Decision(), const std::string& message = std::string())
        : m_decision(decision), m_message(message) {}

    const Decision& decision() const { return m_decision; }
    const std::string message() const { return m_message; }
    
private:
    Decision m_decision;
    std::string m_message;
};

#endif  // __DROP_DECISION_H__
