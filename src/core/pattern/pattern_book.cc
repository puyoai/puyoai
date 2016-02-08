#include "pattern_book.h"

#include <algorithm>
#include <fstream>

using namespace std;

namespace {

ColumnPuyoList diff(const CoreField& before, const BitField& after)
{
    ColumnPuyoList cpl;

    for (int x = 1; x <= 6; ++x) {
        for (int y = before.height(x) + 1; y <= 13; ++y) {
            PuyoColor pc = after.color(x, y);
            if (pc == PuyoColor::EMPTY)
                break;
            cpl.add(x, pc);
        }
    }

    return cpl;
}

} // namespace anonymous

PatternBook::PatternBook() :
    root_(new PatternTree())
{
}

PatternBook::~PatternBook()
{
}

bool PatternBook::load(const string& filename)
{
    ifstream ifs(filename);
    toml::ParseResult result = toml::parse(ifs);

    if (!result.valid()) {
        LOG(ERROR) << result.errorReason;
        return false;
    }
    return loadFromValue(std::move(result.value));
}

bool PatternBook::loadFromString(const string& str, bool ignoreDuplicate)
{
    istringstream ss(str);
    toml::ParseResult result = toml::parse(ss);

    if (!result.valid()) {
        LOG(ERROR) << result.errorReason;
        return false;
    }

    return loadFromValue(result.value, ignoreDuplicate);
}

bool PatternBook::loadFromValue(const toml::Value& patterns, bool ignoreDuplicate)
{
    const toml::Array& vs = patterns.find("pattern")->as<toml::Array>();
    for (const toml::Value& v : vs) {
        string fieldStr;
        for (const auto& s : v.get<toml::Array>("field"))
            fieldStr += s.as<string>();

        string notFieldStr;
        if (const toml::Value* p = v.find("not_field")) {
            for (const auto& s : p->as<toml::Array>())
                notFieldStr += s.as<string>();
        }

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

        FieldBits mustBits;
        if (const toml::Value* p = v.find("precondition")) {
            for (const auto& cp : p->as<toml::Array>()) {
                int x = cp.get<int>(0);
                int y = cp.get<int>(1);
                mustBits.set(x, y);
            }
        }

        FieldBits anyPatternBits(fieldStr, '*');
        FieldBits ironPatternBits(fieldStr, '&');

        int numVariables = 0;
        PatternTree* tree = root_.get();
        PatternTree* mirrorTree = root_.get();
        for (char c = 'A'; c <= 'Z'; ++c) {
            FieldBits bits(fieldStr, c);
            FieldBits allowFieldBits(fieldStr, static_cast<char>(c - 'A' + 'a'));
            FieldBits notBits(notFieldStr, c);

            if (bits.isEmpty()) {
                CHECK(allowFieldBits.isEmpty());
                CHECK(notBits.isEmpty());
                continue;
            }

            notBits.setAll(bits.expandEdge().notmask(bits));
            notBits.unsetAll(anyPatternBits);
            notBits.unsetAll(allowFieldBits);

            numVariables += bits.popcount();

            PatternBit patternBit(bits, notBits);
            tree = tree->put(patternBit);

            PatternBit mirrorPatternBit(bits.mirror(), notBits.mirror());
            mirrorTree = mirrorTree->put(mirrorPatternBit);
        }

        int mirrorIgnitionColumn = ignitionColumn == 0 ? 0 : 7 - ignitionColumn;

        if (ignoreDuplicate) {
            if (!tree->isLeaf())
                tree->setLeaf(name, ironPatternBits, mustBits, ignitionColumn, numVariables, score);
            if (!mirrorTree->isLeaf())
                mirrorTree->setLeaf(name, ironPatternBits.mirror(), mustBits.mirror(), mirrorIgnitionColumn, numVariables, score);
        } else {
            CHECK(tree->setLeaf(name, ironPatternBits, mustBits, ignitionColumn, numVariables, score)) << fieldStr;
            CHECK(mirrorTree->setLeaf(name, ironPatternBits.mirror(), mustBits.mirror(), mirrorIgnitionColumn, numVariables, score)) << fieldStr;
        }
    }

    return true;
}

void PatternBook::complement(const CoreField& originalField,
                                const PatternBook::ComplementCallback& callback) const
{
    complement(originalField, 0, callback);
}

void PatternBook::complement(const CoreField& originalField,
                                int allowedNumUnusedVariables,
                                const ComplementCallback& callback) const
{
    iterate(*root_, originalField, originalField.bitField(), FieldBits(), allowedNumUnusedVariables, 0, callback);
}

void PatternBook::complement(const CoreField& originalField,
                             const FieldBits& ignitionBits,
                             int allowedNumUnusedVariables,
                             const ComplementCallback& callback) const
{
    for (const auto& entry : root_->children_) {
        if (entry.first.varBits() != ignitionBits)
            continue;
        // TODO(mayah): Probably, we don't need to check notBits.
        iterate(*entry.second, originalField, originalField.bitField(),
                entry.first.varBits() & ignitionBits,
                allowedNumUnusedVariables, 0, callback);
    }
}

void PatternBook::iterate(const PatternTree& tree,
                          const CoreField& originalField,
                          const BitField& currentField,
                          const FieldBits& matchedBits,
                          int allowedNumUnusedVariables,
                          int numUnusedVariables,
                          const ComplementCallback& callback) const
{
    if (tree.isLeaf()) {
        if ((tree.patternBookField().mustBits() & originalField.bitField().field13Bits()) == tree.patternBookField().mustBits()) {
            BitField bf(currentField);
            bf.setColorAllIfEmpty(tree.patternBookField().ironBits(), PuyoColor::IRON);
            if (!bf.hasFloatingPuyo()) {
                CoreField cf(bf);
                callback(std::move(cf), diff(originalField, bf), numUnusedVariables, matchedBits, tree.patternBookField());
            }
        }
    }

    FieldBits ojamaBits = currentField.bits(PuyoColor::OJAMA);
    for (const auto& entry : tree.children_) {
        PuyoColor foundColor = PuyoColor::EMPTY;
        bool ok = true;
        FieldBits newMatchedBits(matchedBits);
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            FieldBits matched = entry.first.varBits() & currentField.bits(c);
            if (matched.isEmpty())
                continue;
            if (foundColor != PuyoColor::EMPTY) {
                ok = false;
                break;
            }

            newMatchedBits.setAll(matched);
            foundColor = c;
        }
        if (!ok)
            continue;

        // Check ojama.
        if (!(entry.first.varBits() & ojamaBits).isEmpty())
            continue;

        bool unusedVariableUsed = false;
        if (foundColor == PuyoColor::EMPTY) {
            if (allowedNumUnusedVariables <= numUnusedVariables)
                continue;

            // TODO(mayah): Should check all colors?
            for (PuyoColor c : NORMAL_PUYO_COLORS) {
                if ((entry.first.notBits() & currentField.bits(c)).isEmpty()) {
                    foundColor = c;
                    break;
                }
            }

            if (foundColor == PuyoColor::EMPTY)
                continue;

            unusedVariableUsed = true;
        } else {
            // Check not bits.
            if (!(entry.first.notBits() & currentField.bits(foundColor)).isEmpty())
                continue;
        }

        BitField bf(currentField);
        bf.setColorAll(entry.first.varBits(), foundColor);
        iterate(*entry.second, originalField, bf, newMatchedBits, allowedNumUnusedVariables, unusedVariableUsed ? numUnusedVariables + 1 : numUnusedVariables, callback);
    }
}
