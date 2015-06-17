#ifndef BASE_STRINGS_H_
#define BASE_STRINGS_H_

#ifdef __CYGWIN__
#include <sstream>
#endif
#include <string>
#include <vector>

namespace strings {

// When |s|'s prefix is prefix, true will be returned.
bool isPrefix(const std::string& s, const std::string& prefix);
bool isSuffix(const std::string& s, const std::string& suffix);

// true if |s| contains |t|.
bool contains(const std::string& s, const std::string& t);

// Remove heading and trailing spaces.
std::string trim(const std::string& s);

std::vector<std::string> split(const std::string& s, char separator);

std::string join(const std::vector<std::string>&, const std::string& sep);

}

#ifdef __CYGWIN__

namespace std {

template <typename T>
std::string to_string(const T& v)
{
  std::ostringstream ss;
  ss << v;
  return ss.str();
}

}

#endif

#endif
