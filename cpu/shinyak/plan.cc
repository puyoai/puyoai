#include "plan.h"

#include <sstream>

using namespace std;

std::string Plan::decisionText() const
{
    ostringstream ss;
    
    for (int i = 0; i < numDecisions(); ++i) {
        if (i)
            ss << '-';
        ss << decision(i).toString();
    }

    return ss.str();
}
