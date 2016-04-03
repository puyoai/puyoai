#include "base/file/file.h"

#include <fstream>
#include <string>

using namespace std;

namespace file {

bool readFile(const std::string& filename, std::string* output)
{
    ifstream ifs(filename, ios::in | ios::binary);
    if (!ifs)
        return false;

    output->assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return true;
}

bool writeFile(const std::string& filename, const std::string& output)
{
    ofstream ofs(filename, ios::out | ios::binary);
    if (!ofs)
        return false;

    if (!ofs.write(output.data(), output.size()))
        return false;
    return true;
}

bool copyFile(const std::string& src, const std::string& dst)
{
    ifstream ifs(src, ios::in | ios::binary);
    if (!ifs)
        return false;

    ofstream ofs(dst, ios::out | ios::binary);
    if (!ofs)
        return false;

    ofs << ifs.rdbuf();
    ofs.close();
    ifs.close();

    return true;
}

} // namespace file
