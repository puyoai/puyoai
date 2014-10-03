#include "core/field_pretty_printer.h"

#include <iostream>
#include <sstream>
#include <string>

#include "core/plain_field.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_seq.h"

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
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        PuyoColor c = f.get(x, y);
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

}

// static
string FieldPrettyPrinter::toStringFromMultipleFields(const PlainField& f0, const KumipuyoSeq& seq0,
                                                      const PlainField& f1, const KumipuyoSeq& seq1)
{
    stringstream ss;

    for (int y = PlainField::MAP_HEIGHT - 1; y >= 0; --y) {
        printLine(&ss, f0, y, seq0, CellType::FULL_WITHOUT_COLOR);
        ss << toPuyoString(PuyoColor::EMPTY, CellType::FULL_WITHOUT_COLOR);
        printLine(&ss, f1, y, seq1, CellType::FULL_WITHOUT_COLOR);
        ss << endl;
    }

    return ss.str();
}

// static
void FieldPrettyPrinter::print(const PlainField& f, const KumipuyoSeq& seq)
{
    for (int y = PlainField::MAP_HEIGHT - 1; y >= 0; --y) {
        printLine(&cout, f, y, seq, CellType::FULL_WITH_COLOR);
        cout << endl;
    }
}

// static
void FieldPrettyPrinter::printMultipleFields(const PlainField& f1, const KumipuyoSeq& seq1,
                                             const PlainField& f2, const KumipuyoSeq& seq2)
{
    for (int y = PlainField::MAP_HEIGHT - 1; y >= 0; --y) {
        printLine(&cout, f1, y, seq1, CellType::FULL_WITH_COLOR);
        cout << toPuyoString(PuyoColor::EMPTY, CellType::FULL_WITH_COLOR);
        printLine(&cout, f2, y, seq2, CellType::FULL_WITH_COLOR);
        cout << endl;
    }
}
