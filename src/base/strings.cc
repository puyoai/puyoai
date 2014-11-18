#include "base/strings.h"

using namespace std;

namespace strings {

bool isPrefix(const string& s, const string& prefix)
{
    if (s.size() < prefix.size())
        return false;

    return s.substr(0, prefix.size()) == prefix;
}

string trim(const string& s)
{
    string::size_type p1 = s.find_first_not_of(" ");
    if (p1 == string::npos)
        return string();

    string::size_type p2 = s.find_last_not_of(" ");
    return s.substr(p1, p2 - p1 + 1);
}

vector<string> split(const string& s, char separator)
{
    vector<string> result;
    string::size_type p = 0;
    string::size_type q;
    while ((q = s.find(separator, p)) != string::npos) {
        result.emplace_back(s, p, q - p);
        p = q + 1;
    }

    result.emplace_back(s, p);
    return result;
}

} // namespace strings

