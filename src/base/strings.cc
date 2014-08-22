#include "base/strings.h"

bool isPrefix(const std::string& s, const std::string& prefix)
{
    if (s.size() < prefix.size())
        return false;

    return s.substr(0, prefix.size()) == prefix;
}
