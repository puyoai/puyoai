#include "core/field_pretty_printer.h"

#include <glog/logging.h>

#include <iostream>
#include <sstream>
#include <string>

#include "core/core_field.h"
#include "core/field_constant.h"
#include "core/plain_field.h"
#include "core/kumipuyo_seq.h"
#include "core/puyo_color.h"

using namespace std;

namespace {

enum class CellType {
    HALF,
    FULL_WITHOUT_COLOR,
    FULL_WITH_COLOR,
};

constexpr int ordinal(CellType c) { return static_cast<int>(c); }

const char* const EMPTY_STR[] = { " ", "  ", "  " };
const char* const WALL_STR[] = { "#", "# ", "##" };
const char* const OJAMA_STR[] = { "@", "@ ", "@@" };
const char* const IRON_STR[] = { "&", "& ", "&&" };
const char* const RED_STR[] = { "R", "R ", "\x1b[41m  \x1b[49m" };
const char* const BLUE_STR[] = { "B", "B ", "\x1b[44m  \x1b[49m" };
const char* const GREEN_STR[] = { "G", "G ", "\x1b[42m  \x1b[49m" };
const char* const YELLOW_STR[] = { "Y", "Y ", "\x1b[43m  \x1b[49m" };

const char* toPuyoString(PuyoColor c, CellType cellType)
{
    switch (c) {
    case PuyoColor::EMPTY:
        return EMPTY_STR[ordinal(cellType)];
    case PuyoColor::WALL:
        return WALL_STR[ordinal(cellType)];
    case PuyoColor::OJAMA:
        return OJAMA_STR[ordinal(cellType)];
    case PuyoColor::IRON:
        return IRON_STR[ordinal(cellType)];
    case PuyoColor::RED:
        return RED_STR[ordinal(cellType)];
    case PuyoColor::BLUE:
        return BLUE_STR[ordinal(cellType)];
    case PuyoColor::YELLOW:
        return YELLOW_STR[ordinal(cellType)];
    case PuyoColor::GREEN:
        return GREEN_STR[ordinal(cellType)];
    default:
        CHECK(false) << "Unknown PuyoColor: " << toString(c) << endl;
    }
}

template <typename Stream>
void printLine(Stream* ss, const PlainField& f, int y, const KumipuyoSeq& seq, CellType cellType)
{
    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        PuyoColor c = f.color(x, y);
        *ss << toPuyoString(c, cellType);
    }

    if (y == 11) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 1 ? seq.child(0) : PuyoColor::EMPTY, cellType);
    } else if (y == 10) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 1 ? seq.axis(0) : PuyoColor::EMPTY, cellType);
    } else if (y == 8) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 2 ? seq.child(1) : PuyoColor::EMPTY, cellType);
    } else if (y == 7) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 2 ? seq.axis(1) : PuyoColor::EMPTY, cellType);
    } else if (y == 5) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 3 ? seq.child(2) : PuyoColor::EMPTY, cellType);
    } else if (y == 4) {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(seq.size() >= 3 ? seq.axis(2) : PuyoColor::EMPTY, cellType);
    } else {
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
        *ss << toPuyoString(PuyoColor::EMPTY, cellType);
    }
}

void writeTo(ostream* os,
             std::initializer_list<PlainField> fields,
             std::initializer_list<KumipuyoSeq> seqs,
             CellType cellType)
{
    for (int y = FieldConstant::MAP_HEIGHT - 1; y >= 0; --y) {
        auto seqIt = seqs.begin();
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            if (it != fields.begin())
                *os << toPuyoString(PuyoColor::EMPTY, cellType);
            if (seqIt != seqs.end()) {
                printLine(os, *it, y, *seqIt, cellType);
                ++seqIt;
            } else {
                printLine(os, *it, y, KumipuyoSeq(), cellType);
            }
        }
        *os << endl;
    }
}

}

// static
string FieldPrettyPrinter::toStringFromMultipleFields(initializer_list<PlainField> fields,
                                                      initializer_list<KumipuyoSeq> seq)
{
    ostringstream ss;
    writeTo(&ss, fields, seq, CellType::FULL_WITHOUT_COLOR);
    return ss.str();
}

// static
void FieldPrettyPrinter::print(const PlainField& f, const KumipuyoSeq& seq)
{
    writeTo(&cout, { f }, { seq }, CellType::FULL_WITH_COLOR);
}

// static
void FieldPrettyPrinter::printMultipleFields(initializer_list<PlainField> fields,
                                             initializer_list<KumipuyoSeq> seqs)
{
    writeTo(&cout, fields, seqs, CellType::FULL_WITH_COLOR);
}
