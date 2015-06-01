#include "base/strings.h"

#include <sstream>

using namespace std;

namespace strings {

bool isPrefix(const string& s, const string& prefix)
{
    if (s.size() < prefix.size())
        return false;

    return s.substr(0, prefix.size()) == prefix;
}

bool isSuffix(const string& s, const string& suffix)
{
    if (s.size() < suffix.size())
        return false;

    return s.substr(s.size() - suffix.size()) == suffix;
}

bool contains(const string& s, const string& t)
{
    return s.find(t) != string::npos;
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

string join(const vector<string>& strs, const string& sep)
{
    if (strs.size() == 0)
        return string();

    ostringstream os;
    os << strs[0];
    for (size_t i = 1; i < strs.size(); ++i) {
        os << sep << strs[i];
    }
    return os.str();
}

} // namespace strings
