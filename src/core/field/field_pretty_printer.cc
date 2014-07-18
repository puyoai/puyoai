#include "core/field/field_pretty_printer.h"

#include <iostream>
#include <sstream>
#include <string>

#include "core/plain_field.h"
#include "core/kumipuyo.h"

using namespace std;

namespace {
const char RED_COLOR[] = "\x1b[41m";
const char BLUE_COLOR[] = "\x1b[44m";
const char GREEN_COLOR[] = "\x1b[42m";
const char YELLOW_COLOR[] = "\x1b[43m";
const char BLACK_COLOR[] = "\x1b[49m";

string toPuyoColorString(PuyoColor c)
{
    stringstream ss;
    switch (c) {
    case PuyoColor::EMPTY:
        ss << "  ";
        break;
    case PuyoColor::WALL:
        ss << "##";
        break;
    case PuyoColor::OJAMA:
        ss << "@@";
        break;
    case PuyoColor::RED:
        ss << RED_COLOR << "  " << BLACK_COLOR;
        break;
    case PuyoColor::BLUE:
        ss << BLUE_COLOR << "  " << BLACK_COLOR;
        break;
    case PuyoColor::YELLOW:
        ss << YELLOW_COLOR << "  " << BLACK_COLOR;
        break;
    case PuyoColor::GREEN:
        ss << GREEN_COLOR << "  " << BLACK_COLOR;
        break;
    }

    return ss.str();
}

void printLine(const PlainField& f, int y, const KumipuyoSeq& seq)
{
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        PuyoColor c = f.get(x, y);
        cout << toPuyoColorString(c);
    }

    if (y == 11) {
        cout << toPuyoColorString(PuyoColor::EMPTY);
        cout << toPuyoColorString(seq.child(0));
    } else if (y == 10) {
        cout << toPuyoColorString(PuyoColor::EMPTY);
        cout << toPuyoColorString(seq.axis(0));
    } else if (y == 8) {
        cout << toPuyoColorString(PuyoColor::EMPTY);
        cout << toPuyoColorString(seq.child(1));
    } else if (y == 7) {
        cout << toPuyoColorString(PuyoColor::EMPTY);
        cout << toPuyoColorString(seq.axis(1));
    } else {
        cout << toPuyoColorString(PuyoColor::EMPTY);
        cout << toPuyoColorString(PuyoColor::EMPTY);
    }
}

}

// static
void FieldPrettyPrinter::print(const PlainField& f, const KumipuyoSeq& seq)
{
    for (int y = PlainField::MAP_HEIGHT - 1; y >= 0; --y) {
        printLine(f, y, seq);
        cout << endl;
    }
}

// static
void FieldPrettyPrinter::printMultipleFields(
    const PlainField& f1, const KumipuyoSeq& seq1,
    const PlainField& f2, const KumipuyoSeq& seq2)
{
    for (int y = PlainField::MAP_HEIGHT - 1; y >= 0; --y) {
        printLine(f1, y, seq1);
        cout << toPuyoColorString(PuyoColor::EMPTY);
        printLine(f2, y, seq2);
        cout << endl;
    }
}
