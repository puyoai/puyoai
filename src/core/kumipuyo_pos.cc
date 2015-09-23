#include "core/kumipuyo_pos.h"

#include "base/strings.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

KumipuyoPos::KumipuyoPos(const std::string& str)
{
  std::string term;
  std::istringstream iss(str);

  while (iss >> term) {
    const auto& v = strings::split(term, '=');
    if (v.size() == 2) {
      if (v[0] == "x") {
        x = atoi(v[1].c_str());
      } else if (v[0] == "y") {
        y = atoi(v[1].c_str());
      } else if (v[0] == "r") {
        r = atoi(v[1].c_str());
      }
    }
  }
}

string KumipuyoPos::toDebugString() const
{
    char buf[256];
    snprintf(buf, 256, "<y=%d,x=%d,r=%d>", y, x, r);
    return std::string(buf);
}

std::string KumipuyoPos::toString() const
{
  stringstream ss;
  ss << "x=" << x << " "
     << "y=" << y << " "
     << "r=" << r;
  return ss.str();
}
