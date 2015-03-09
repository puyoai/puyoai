#include "pattern_book.h"

#include <algorithm>
#include <fstream>

using namespace std;

namespace {

const int MAX_UNUSED_VARIABLES = 1;

vector<Position> findIgnitionPositions(const FieldPattern& pattern)
{
    Position positions[FieldConstant::MAP_WIDTH * FieldConstant::MAP_HEIGHT];

    FieldBitField checked;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (checked(x, y))
                continue;
            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;
            char c = pattern.variable(x, y);
            CHECK('A' <= c && c <= 'Z');
            Position* p = pattern.fillSameVariablePositions(x, y, c, positions, &checked);
            if (p - positions >= 4) {
                std::sort(positions, p);
                return vector<Position>(positions, p);
            }
        }
    }

    CHECK(false) << "there is no 4-connected variables." << pattern.toDebugString();
    return vector<Position>();
}

} // anonymous namespace

PatternBookField::PatternBookField(const string& field, const string& name, int ignitionColumn, double score) :
    pattern_(field),
    name_(name),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}

PatternBookField::PatternBookField(const FieldPattern& pattern, const string& name, int ignitionColumn, double score) :
    pattern_(pattern),
    name_(name),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}

bool PatternBook::load(const string& filename)
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

bool PatternBook::loadFromString(const string& str)
{
    istringstream ss(str);
    toml::Parser parser(ss);
    toml::Value v = parser.parse();
    if (!v.valid()) {
        LOG(ERROR) << parser.errorReason();
        return false;
    }

    return loadFromValue(v);
}

bool PatternBook::loadFromValue(const toml::Value& patterns)
{
    CHECK(fields_.empty());
    CHECK(index_.empty());

    const toml::Array& vs = patterns.find("pattern")->as<toml::Array>();
    for (const toml::Value& v : vs) {
        string str;
        for (const auto& s : v.get<toml::Array>("field"))
            str += s.as<string>();

        string name;
        if (const toml::Value* p = v.find("name")) {
            name = p->as<string>();
        }

        int ignitionColumn = 0;
        if (const toml::Value* p = v.find("ignition")) {
            ignitionColumn = p->as<int>();
            CHECK(1 <= ignitionColumn && ignitionColumn <= 6) << ignitionColumn;
        }

        double score = 0;
        if (const toml::Value* p = v.find("score")) {
            if (p->is<int>())
                score = p->as<int>();
            else if (p->is<double>())
                score = p->as<double>();
            else
                CHECK(false);
        }

        PatternBookField pbf(str, name, ignitionColumn, score);

        if (const toml::Value* p = v.find("precondition")) {
            for (const auto& cp : p->as<toml::Array>()) {
                int x = cp.get<int>(0);
                int y = cp.get<int>(1);
                CHECK(pbf.pattern().type(x, y) == PatternType::VAR);
                pbf.mutablePattern()->setType(x, y, PatternType::MUST_VAR);
            }
        }

        fields_.push_back(pbf);
        fields_.push_back(pbf.mirror());
    }

    index_.clear();
    for (int i = 0; i < static_cast<int>(fields_.size()); ++i)
        index_.emplace(fields_[i].ignitionPositions(), i);

    return true;
}

pair<PatternBook::IndexIterator, PatternBook::IndexIterator>
PatternBook::find(const vector<Position>& ignitionPositions) const
{
    return index_.equal_range(ignitionPositions);
}

void PatternBook::iteratePossibleRensas(const CoreField& originalField,
                                        const vector<int>& matchableIds,
                                        int maxIteration,
                                        const Callback& callback) const
{
    DCHECK_GE(maxIteration, 1);

    const CoreField::SimulationContext originalContext = CoreField::SimulationContext::fromField(originalField);
    const RensaDetectorStrategy strategy = RensaDetectorStrategy(RensaDetectorStrategy::Mode::DROP, 2, 2, false);

    // --- Iterate with complementing
    for (const int id : matchableIds) {
        const PatternBookField& pbf = patternBookField(id);
        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(originalField, MAX_UNUSED_VARIABLES, &cpl);
        if (!complementResult.success)
            continue;
        if (complementResult.numFilledUnusedVariables > 0)
            continue;

        CoreField cf(originalField);
        bool ok = true;
        bool foundFirePuyo = false;
        ColumnPuyo firePuyo;
        ColumnPuyoList keyPuyos;
        for (const ColumnPuyo& cp : cpl) {
            if (!cf.dropPuyoOn(cp.x, cp.color)) {
                ok = false;
                break;
            }
            if (foundFirePuyo) {
                if (cp.x == pbf.ignitionColumn()) {
                    keyPuyos.add(firePuyo);
                    firePuyo = cp;
                } else if (!keyPuyos.add(cp)) {
                    ok = false;
                    break;
                }
            } else {
                if (cp.x == pbf.ignitionColumn()) {
                    foundFirePuyo = true;
                    firePuyo = cp;
                } else {
                    keyPuyos.add(cp);
                }
            }
        }

        if (!ok || !foundFirePuyo)
            continue;

        CoreField::SimulationContext context(originalContext);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        int restUnusedVariables = MAX_UNUSED_VARIABLES - complementResult.numFilledUnusedVariables;
        double ratio = 0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            for (const auto& cp : cpl) {
                if (cp.color != PuyoColor::OJAMA)
                    ++count;
            }
            double d = pbf.numVariables() - count;
            ratio = d / pbf.numVariables();
        }
        iteratePossibleRensasInternal(originalField, strategy, 1,
                                      cf, context, firePuyo, keyPuyos,
                                      maxIteration - 1, restUnusedVariables, pbf.score() * ratio, callback);
    }

    // --- Iterate without complementing.
    auto detectCallback = [&](CoreField* cf, const ColumnPuyoList& cpl) {
        if (cpl.size() == 0)
            return;

        bool first = true;
        ColumnPuyo firePuyo;
        ColumnPuyoList keyPuyos;
        for (const ColumnPuyo& cp : cpl) {
            if (first) {
                firePuyo = cp;
                first = false;
                continue;
            }

            keyPuyos.add(firePuyo);
            firePuyo = cp;
        }

        iteratePossibleRensasInternal(originalField, strategy, 0, *cf, originalContext,
                                      firePuyo, keyPuyos, maxIteration - 1, 0, 0, callback);

    };

    bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detect(originalField, strategy, PurposeForFindingRensa::FOR_FIRE, prohibits, detectCallback);
}

void PatternBook::iteratePossibleRensasInternal(const CoreField& original,
                                                const RensaDetectorStrategy& strategy,
                                                int currentChains,
                                                const CoreField& currentField,
                                                const CoreField::SimulationContext& currentFieldContext,
                                                const ColumnPuyo& firePuyo,
                                                const ColumnPuyoList& originalKeyPuyos,
                                                int restIteration,
                                                int restUnusedVariables,
                                                double patternScore,
                                                const Callback& callback) const
{
    // if currentField does not erase anything...
    if (!currentField.rensaWillOccurWithContext(currentFieldContext)) {
        bool prohibits[FieldConstant::MAP_WIDTH] {};
        if (!checkRensa(original, strategy, currentChains, firePuyo, originalKeyPuyos, patternScore, prohibits, callback))
            return;
        if (restIteration <= 0)
            return;
        auto detectCallback = [&](CoreField* cf, const ColumnPuyoList& cpl) {
            if (cpl.size() == 0)
                return;

            bool ok = true;
            ColumnPuyoList keyPuyos(originalKeyPuyos);
            for (const ColumnPuyo& cp : cpl) {
                if (!keyPuyos.add(cp)) {
                    ok = false;
                    break;
                }
            }

            if (!ok)
                return;

            CoreField::SimulationContext context(currentFieldContext);
            int score = cf->vanishDrop(&context);
            if (score == 0)
                return;

            iteratePossibleRensasInternal(original, strategy, currentChains + 1, *cf, context,
                                          firePuyo, keyPuyos, restIteration - 1, restUnusedVariables,
                                          patternScore, callback);
        };

        RensaDetector::detect(currentField, strategy, PurposeForFindingRensa::FOR_KEY, prohibits, detectCallback);
        return;
    }

    // With complement.
    std::vector<Position> ignitionPositions = currentField.erasingPuyoPositions(currentFieldContext);
    if (ignitionPositions.empty())
        return;

    std::sort(ignitionPositions.begin(), ignitionPositions.end());
    std::pair<IndexIterator, IndexIterator> p = this->find(ignitionPositions);
    bool needsToProceedWithoutComplement = true;
    for (IndexIterator it = p.first; it != p.second; ++it) {
        const PatternBookField& pbf = patternBookField(it->second);

        ColumnPuyoList cpl;
        ComplementResult complementResult = pbf.complement(currentField, restUnusedVariables, &cpl);
        if (!complementResult.success)
            continue;

        if (cpl.size() == 0) {
            needsToProceedWithoutComplement = false;

            CoreField cf(currentField);
            CoreField::SimulationContext context(currentFieldContext);
            int score = cf.vanishDrop(&context);
            CHECK(score > 0) << score;

            iteratePossibleRensasInternal(original, strategy, currentChains + 1,
                                          cf, context, firePuyo, originalKeyPuyos,
                                          restIteration, restUnusedVariables,
                                          patternScore + pbf.score(), callback);
            continue;
        }

        if (restIteration <= 0)
            continue;

        CoreField cf(currentField);
        ColumnPuyoList keyPuyos(originalKeyPuyos);

        bool ok = true;
        for (const ColumnPuyo& cp : cpl) {
            if (cp.x == firePuyo.x) {
                ok = false;
                break;
            }
            if (!keyPuyos.add(cp)) {
                ok = false;
                break;
            }
            if (!cf.dropPuyoOn(cp.x, cp.color)) {
                ok = false;
                break;
            }
        }

        if (!ok)
            continue;

        CoreField::SimulationContext context(currentFieldContext);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;
        double ratio = 0.0;
        if (pbf.numVariables() > 0) {
            int count = 0;
            for (const auto& cp : cpl) {
                if (cp.color != PuyoColor::OJAMA)
                    ++count;
            }
            double d = pbf.numVariables() - count;
            ratio = d / pbf.numVariables();
        }

        iteratePossibleRensasInternal(original, strategy, currentChains + 1,
                                      cf, context, firePuyo, keyPuyos,
                                      restIteration - 1,
                                      restUnusedVariables - complementResult.numFilledUnusedVariables,
                                      patternScore + pbf.score() * ratio, callback);
    }

    // proceed one without complementing.
    if (needsToProceedWithoutComplement) {
        CoreField cf(currentField);
        CoreField::SimulationContext context(currentFieldContext);
        int score = cf.vanishDrop(&context);
        CHECK(score > 0) << score;

        iteratePossibleRensasInternal(original, strategy, currentChains + 1,
                                      cf, context, firePuyo, originalKeyPuyos,
                                      restIteration, restUnusedVariables, patternScore, callback);
    }

}

bool PatternBook::checkRensa(const CoreField& originalField,
                             const RensaDetectorStrategy& strategy,
                             int currentChains,
                             const ColumnPuyo& firePuyo,
                             const ColumnPuyoList& keyPuyos,
                             double patternScore,
                             bool prohibits[FieldConstant::MAP_WIDTH],
                             const Callback& callback) const
{
    CoreField cf(originalField);
    CoreField::SimulationContext context = CoreField::SimulationContext::fromField(originalField);

    const int maxHeight = strategy.allowsPuttingKeyPuyoOn13thRow() ? 13 : 12;

    for (const auto& cp : keyPuyos) {
        if (!cf.dropPuyoOn(cp.x, cp.color))
            return false;
        if (cf.height(cp.x) > maxHeight)
            return false;
    }

    // If rensa occurs after adding key puyos, this is invalid.
    if (cf.rensaWillOccurWithContext(context))
        return false;

    context.updateFromField(cf);

    if (!cf.dropPuyoOn(firePuyo.x, firePuyo.color))
        return false;

    RensaTrackResult trackResult;
    RensaResult rensaResult = cf.simulateWithContext(&context, &trackResult);
    if (rensaResult.chains != currentChains)
        return false;

    ColumnPuyoList firePuyos;
    if (!firePuyos.add(firePuyo))
        return false;

    callback(cf, rensaResult, keyPuyos, firePuyos, trackResult, patternScore);

    RensaDetector::makeProhibitArray(rensaResult, trackResult, originalField,
                                     firePuyos, prohibits);
    return true;
}
