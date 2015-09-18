#include "base/file.h"

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace file {

string joinPath(const string& lhs, const string& rhs)
{
    if (lhs.empty())
        return rhs;
    if (rhs.empty())
        return lhs;

    if (lhs.back() == '/' && rhs.front() == '/')
        return lhs + rhs.substr(1);

    if (lhs.back() != '/' && rhs.front() != '/')
        return lhs + "/" + rhs;

    return lhs + rhs;
}

string joinPath(const string& p1, const string& p2, const string& p3)
{
    return joinPath(joinPath(p1, p2), p3);
}

string joinPath(const string& p1, const string& p2, const string& p3, const string& p4)
{
    return joinPath(joinPath(p1, p2, p3), p4);
}

bool isDirectory(const string& path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) < 0)
        return false;

    return S_ISDIR(sb.st_mode);
}

bool listFiles(const string& path, vector<string>* files)
{
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return false;

    while (true) {
        struct dirent* dent = readdir(dir);
        if (!dent)
            break;
        files->push_back(dent->d_name);
    }

    if (closedir(dir) < 0)
        return false;

    return true;
}

} // namespace file
