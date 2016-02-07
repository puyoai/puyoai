// This file does compile if -mavx2 is specified or -mnative is specified and CPU has AVX2.
#ifdef __AVX2__

#include <sstream>

#include "core/field_bits_256.h"

using namespace std;

string FieldBits256::toString() const
{
    stringstream ss;
    for (int y = 15; y >= 0; --y) {
        for (int x = 0; x < 8; ++x) {
            ss << (get(HighLow::HIGH, x, y) ? '1' : '0');
        }
        ss << "   ";
        for (int x = 0; x < 8; ++x) {
            ss << (get(HighLow::LOW, x, y) ? '1' : '0');
        }
        ss << endl;
    }

    return ss.str();
}

#endif // __AVX2__
