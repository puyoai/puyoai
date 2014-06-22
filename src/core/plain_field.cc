#include "core/plain_field.h"

using namespace std;

PlainField::PlainField()
{
    initialize();
}

PlainField::PlainField(const string& url)
{
    initialize();

    std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int data_starts_at = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= data_starts_at; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = puyoColorOf(url[i]);
        unsafeSet(x, y, c);
        counter++;
    }
}

PlainField::PlainField(const PlainField& original)
{
    memcpy(field_, original.field_, sizeof(field_));
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
    std::ostringstream s;
    for (int y = 13; y >= 1; --y) {
        for (int x = 1; x <= WIDTH; ++x) {
            s << charOfPuyoColor(get(x, y), charIfEmpty);
        }
    }

    return s.str();
}
