#include "estimated_rensa_info.h"

#include <iostream>
#include <sstream>

using namespace std;

string EstimatedRensaInfo::toString() const
{
    char buf[80];
    sprintf(buf, "totalFrames, chains, score, framesToIgnite = %d, %d, %d, %d", totalFrames(), chains(), score(), framesToIgnite());
    return buf;
}

string EstimatedRensaInfoTree::toString() const
{
    ostringstream oss;
    dumpTo(0, &oss);

    return oss.str();
}

void EstimatedRensaInfoTree::dump(int depth) const
{
    dumpTo(depth, &cout);
}

void EstimatedRensaInfoTree::dumpTo(int depth, ostream* os) const
{
    for (int i = 0; i < depth * 2; ++i)
        *os << ' ';
    *os << estimatedRensaInfo.toString() << endl;

    for (const auto& t : children)
        t.dumpTo(depth + 1, os);
}
