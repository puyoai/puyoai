#include "core/field_bits.h"

#include <sstream>

using namespace std;

std::string FieldBits::toString() const
{
    stringstream ss;
    for (int y = 15; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            ss << (get(x, y) ? '1' : '0');
        }
        ss << endl;
    }

    return ss.str();
}
