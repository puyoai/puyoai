#ifndef CORE_PATTERN_DECISION_BOOK_H_
#define CORE_PATTERN_DECISION_BOOK_H_

#include <toml/toml.h>

#include <map>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "core/decision.h"
#include "core/pattern/field_pattern.h"

class BijectionMatcher;
class CoreField;
class Kumipuyo;
class KumipuyoSeq;

class DecisionBookField {
public:
    DecisionBookField(const std::vector<std::string>& field,
                      std::map<std::string, Decision>&& decisions);

    Decision nextDecision(const CoreField&, const KumipuyoSeq&) const;

private:
    bool matchNext(BijectionMatcher*, const std::string& nextPattern, const Kumipuyo& next1, const Kumipuyo& next2) const;

    FieldPattern pattern_;
    std::map<std::string, Decision> decisions_;
};

// DecisionBook is a book to return a fixed Decision from the given field and kumipuyo sequence.
// It is useful to make a book in the very early phase.
class DecisionBook : noncopyable {
public:
    DecisionBook();
    explicit DecisionBook(const std::string& filename);

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    // Finds next decision. If next decision is not found, invalid Decision will be returned.
    Decision nextDecision(const CoreField&, const KumipuyoSeq&) const;

private:
    void makeFieldFromValue(const CoreField&, const std::string&, const toml::Value&);

    std::vector<DecisionBookField> fields_;
};

#endif // CPU_MAYAH_DECISION_BOOK_H_
