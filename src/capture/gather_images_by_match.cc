#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/file.h"
#include "base/strings.h"

using namespace std;

bool listBMPsRecursively(const std::string& root, std::vector<std::string>* files)
{
    vector<string> tmp_files;
    if (!file::listFiles(root, &tmp_files))
        return false;

    for (const auto& f : tmp_files) {
        string p = file::joinPath(root, f);

        if (strings::hasSuffix(f, ".bmp")) {
            files->push_back(p);
            continue;
        }

        if (!file::isDirectory(p)) {
            LOG(INFO) << "Unknown file: " << p << endl;
            continue;
        }


        if (!listBMPsRecursively(p, files)) {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <root directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // list files.
    vector<string> files;
    CHECK(listBMPsRecursively(argv[1], &files));
    std::sort(files.begin(), files.end());

    for (const auto& f : files) {
        cout << f << endl;
    }

    CHECK(false) << "Not implemented yet";
    return 0;
}
