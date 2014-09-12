#include "core/plain_field.h"

#include <sstream>

using namespace std;

PlainField::PlainField()
{
    initialize();
}

PlainField::PlainField(const string& url)
{
    initialize();

    string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int dataStartsAt = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= dataStartsAt; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = toPuyoColor(url[i]);
        unsafeSet(x, y, c);
        counter++;
    }
}

void PlainField::initialize()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            unsafeSet(x, y, PuyoColor::EMPTY);
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        unsafeSet(x, 0, PuyoColor::WALL);
        unsafeSet(x, MAP_HEIGHT - 1, PuyoColor::WALL);
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        unsafeSet(0, y, PuyoColor::WALL);
        unsafeSet(MAP_WIDTH - 1, y, PuyoColor::WALL);
    }
}

string PlainField::toString(char charIfEmpty) const
{
    ostringstream ss;
    for (int y = 14; y >= 1; --y) {
        for (int x = 1; x <= WIDTH; ++x) {
            ss << toChar(get(x, y), charIfEmpty);
        }
    }

    return ss.str();
}
