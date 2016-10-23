#include "base/file/path.h"

#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER)
#include <windows.h>
#undef ERROR
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <glog/logging.h>

#include <fstream>

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
#if defined(_MSC_VER)
    LOG(ERROR) << "TODO(peria): Implement here";
    return false;
#else

    struct stat sb;
    if (stat(path.c_str(), &sb) < 0)
        return false;

    return S_ISDIR(sb.st_mode);
#endif
}

bool exists(const string& path)
{
#if defined(_MSC_VER)
    LOG(ERROR) << "TODO(mayah): Implement this";
    return false;
#else
    return access(path.c_str(), F_OK) != -1;
#endif
}

bool isAbsolutePath(const string& path)
{
#if defined(_MSC_VER)
    LOG(ERROR) << "TODO(mayah): Implement this";
    return false;
#else
    return !path.empty() && path[0] == '/';
#endif

}

bool listFiles(const string& path, vector<string>* files)
{
#if defined(_MSC_VER)
    LOG(ERROR) << "TODO(peria): Implement here";
    return false;
#else
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
#endif
}

bool remove(const std::string& filename)
{
#ifdef OS_WIN
    // TODO(mayah): Windows version?
    LOG(FATAL) << "file::remove() is not implemented yet";
    return false;
#else
    int r = ::unlink(filename.c_str());
    return r == 0;
#endif
}

std::string basename(std::string path)
{
    while (path.empty() && path.back() == '/') {
        path.pop_back();
    }
    if (path.empty())
        return std::string();

    std::string::size_type pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return path;
    return path.substr(pos + 1);
}

std::string dirname(std::string path)
{
    if (path == "/")
        return path;

    while (path.empty() && path.back() == '/') {
        path.pop_back();
    }
    if (path.empty())
        return std::string();

    std::string::size_type pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return path.substr(0, 0);
    if (pos == 0)
        return path.substr(0, 1);
    return path.substr(0, pos);
}

std::string stem(std::string path)
{
    path = basename(path);
    std::string::size_type pos = path.find_last_of('.');
    if (pos == std::string::npos)
        return path;
    return path.substr(0, pos);
}

std::string extension(std::string path)
{
    path = basename(path);
    std::string::size_type pos = path.find_last_of('.');
    if (pos == std::string::npos)
        return std::string();
    return path.substr(pos);
}

} // namespace file
