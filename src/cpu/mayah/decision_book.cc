#include "decision_book.h"

#include <glog/logging.h>
#include <toml/toml.h>

#include <algorithm>
#include <fstream>
#include <utility>

#include "base/strings.h"
#include "core/algorithm/bijection_matcher.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_seq.h"

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

DecisionBookField::DecisionBookField(const vector<string>& field, map<string, Decision>&& decisions) :
    pattern_(strings::join(field, "")),
    decisions_(move(decisions))
{
}

Decision DecisionBookField::nextDecision(const CoreField& cf, const KumipuyoSeq& seq) const
{
    BijectionMatcher matcher;
    if (!matcher.match(pattern_, cf))
        return Decision();

    const Kumipuyo& kp1 = seq.get(0);
    const Kumipuyo& kp2 = seq.get(1);

    // Field matched. check next sequence.
    for (const auto& entry : decisions_) {
        if (matchNext(&matcher, entry.first, kp1, kp2))
            return entry.second;

        if (!kp2.isRep() && matchNext(&matcher, entry.first, kp1, kp2.reverse()))
            return entry.second;

        if (!kp1.isRep() && matchNext(&matcher, entry.first, kp1.reverse(), kp2))
            return entry.second.reverse();

        if (!kp1.isRep() && !kp2.isRep() && matchNext(&matcher, entry.first, kp1.reverse(), kp2.reverse()))
            return entry.second.reverse();
    }

    return Decision();
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
    toml::Parser parser(ifs);
    toml::Value v = parser.parse();
    if (!v.valid()) {
        LOG(ERROR) << parser.errorReason();
        return false;
    }

    return loadFromValue(std::move(v));
}

bool DecisionBook::loadFromString(const string& str)
{
    istringstream iss(str);
    toml::Parser parser(iss);
    toml::Value v = parser.parse();
    if (!v.valid()) {
        LOG(ERROR) << parser.errorReason();
        return false;
    }

    return loadFromValue(v);
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
        map<string, Decision> m;
        for (const auto& e : v.as<toml::Table>()) {
            if (e.first == "field")
                continue;
            m[e.first] = makeDecision(e.second);
        }

        fields_.emplace_back(f, std::move(m));
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
