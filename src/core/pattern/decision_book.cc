#include "decision_book.h"

#include <glog/logging.h>
#include <toml/toml.h>

#include <algorithm>
#include <fstream>
#include <utility>

#include "base/strings.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_seq.h"
#include "core/pattern/bijection_matcher.h"

using namespace std;

namespace {

Decision makeDecision(const toml::Value& v)
{
    const toml::Array ary = v.as<toml::Array>();
    int x = ary[0].as<int>();
    int r = ary[1].as<int>();
    return Decision(x, r);
}

} // namespace anonymous

DecisionBookField::DecisionBookField(const vector<string>& field, map<string, Decision>&& decisions1, map<string, Decision>&& decisions2) :
    pattern_(strings::join(field, "")),
    decisions1_(move(decisions1)),
    decisions2_(move(decisions2))
{
}

Decision DecisionBookField::nextDecision(const CoreField& cf, const KumipuyoSeq& seq) const
{
    BijectionMatcher matcher;
    if (!matcher.match(pattern_, cf))
        return Decision();

    // Now it is guaranteed that field is matched.
    const Kumipuyo& kp1 = seq.get(0);
    const Kumipuyo& kp2 = seq.get(1);

    // Check sequence with 2 Tsumos.
    for (const auto& entry : decisions2_) {
        if (matchNext(&matcher, entry.first, kp1, kp2))
            return entry.second;
        if (!kp2.isRep() && matchNext(&matcher, entry.first, kp1, kp2.reverse()))
            return entry.second;

        if (kp1.isRep())
            continue;

        if (matchNext(&matcher, entry.first, kp1.reverse(), kp2))
            return entry.second.reverse();
        if (!kp2.isRep() && matchNext(&matcher, entry.first, kp1.reverse(), kp2.reverse()))
            return entry.second.reverse();
    }

    // Check sequence with 1 Tsumo.
    for (const auto& entry : decisions1_) {
        if (matchNext(&matcher, entry.first, kp1))
            return entry.second;
        if (kp1.isRep())
            continue;
        if (matchNext(&matcher, entry.first, kp1.reverse()))
            return entry.second.reverse();
    }

    return Decision();
}

bool DecisionBookField::matchNext(BijectionMatcher* matcher,
                                  const string& nextPattern,
                                  const Kumipuyo& next1) const
{
    DCHECK_EQ(nextPattern.size(), 2UL) << nextPattern;
    BijectionMatcher bm(*matcher);

    if (!bm.match(nextPattern[0], next1.axis))
        return false;
    if (!bm.match(nextPattern[1], next1.child))
        return false;

    *matcher = bm;
    return true;
}

bool DecisionBookField::matchNext(BijectionMatcher* matcher,
                                  const string& nextPattern,
                                  const Kumipuyo& next1,
                                  const Kumipuyo& next2) const
{
    DCHECK_EQ(nextPattern.size(), 4UL) << nextPattern;
    BijectionMatcher bm(*matcher);

    if (!bm.match(nextPattern[0], next1.axis))
        return false;
    if (!bm.match(nextPattern[1], next1.child))
        return false;
    if (!bm.match(nextPattern[2], next2.axis))
        return false;
    if (!bm.match(nextPattern[3], next2.child))
        return false;

    *matcher = bm;
    return true;
}

DecisionBook::DecisionBook()
{
}

DecisionBook::DecisionBook(const string& filename)
{
    CHECK(load(filename));
}

bool DecisionBook::load(const string& filename)
{
    ifstream ifs(filename);
    toml::ParseResult result = toml::parse(ifs);
    if (!result.valid()) {
        LOG(ERROR) << result.errorReason;
        return false;
    }

    return loadFromValue(std::move(result.value));
}

bool DecisionBook::loadFromString(const string& str)
{
    istringstream iss(str);
    toml::ParseResult result = toml::parse(iss);
    if (!result.valid()) {
        LOG(ERROR) << result.errorReason;
        return false;
    }

    return loadFromValue(result.value);
}

bool DecisionBook::loadFromValue(const toml::Value& book)
{
    if (!book.valid())
        return false;

    const toml::Array& vs = book.find("book")->as<toml::Array>();
    fields_.reserve(vs.size());
    for (const toml::Value& v : vs) {
        vector<string> f;
        for (const auto& s : v.get<toml::Array>("field"))
            f.push_back(s.as<string>());
        map<string, Decision> m1;
        map<string, Decision> m2;
        for (const auto& e : v.as<toml::Table>()) {
            if (e.first == "field")
                continue;
            switch (e.first.size()) {
            case 2:
                m1[e.first] = makeDecision(e.second);
                break;
            case 4:
                m2[e.first] = makeDecision(e.second);
                break;
            default:
                CHECK(false) << "Invalid Tsumo assumption: " << e.first;
            }
        }
        fields_.emplace_back(f, std::move(m1), std::move(m2));
    }

    return true;
}

Decision DecisionBook::nextDecision(const CoreField& cf, const KumipuyoSeq& seq) const
{
    for (const auto& f : fields_) {
        Decision decision = f.nextDecision(cf, seq);
        if (decision.isValid())
            return decision;
    }

    return Decision();
}
