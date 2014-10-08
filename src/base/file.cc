#include "base/file.h"

#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace file {

std::string joinPath(const std::string& lhs, const std::string& rhs)
{
    string s = lhs + "/" + rhs;
    char* x = realpath(s.c_str(), nullptr);
    if (!x)
        return string();

    string result(x);
    free(x);
    return result;
}

bool isDirectory(const std::string& path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) < 0)
        return false;

    return S_ISDIR(sb.st_mode);
}

} // namespace file
