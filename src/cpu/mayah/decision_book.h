#ifndef CPU_MAYAH_DECISION_BOOK_H_
#define CPU_MAYAH_DECISION_BOOK_H_

#include <string>
#include <map>

#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/algorithm/pattern_field.h"
#include "core/decision.h"
#include "core/field_constant.h"
#include "core/puyo_color.h"

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

    PatternField patternField_;
    std::map<std::string, Decision> decisions_;
};

class DecisionBook : noncopyable {
public:
    DecisionBook();
    explicit DecisionBook(const std::string& filename);

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    // Finds next decision. If next decision is not found, invalid Decision will be returned.
    Decision nextDecision(const CoreField&, const KumipuyoSeq&);

private:
    void makeFieldFromValue(const CoreField&, const std::string&, const toml::Value&);

    std::vector<DecisionBookField> fields_;
};

#endif
