#include "base/file/file.h"

#include <fstream>
#include <string>

using namespace std;

namespace file {

bool readFile(const std::string& filename, string* output)
{
    ifstream ifs(filename, ios::in | ios::binary);
    if (!ifs)
        return false;

    output->assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return true;
}

} // namespace file
