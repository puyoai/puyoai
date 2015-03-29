#include "core/rensa_result.h"

#include <sstream>

using namespace std;

string RensaResult::toString() const
{
    stringstream ss;
    ss << "chains=" << chains
       << " score=" << score
       << " frames=" << frames
       << " quick=" << (quick ? "true" : "false");
    return ss.str();
}
