#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "base/file/path.h"
#include "base/strings.h"
#include "capture/ac_analyzer.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

bool listBMPsRecursively(const std::string& root, std::vector<std::string>* files)
{
    cout << root << endl;

    vector<string> tmp_files;
    if (!file::listFiles(root, &tmp_files))
        return false;

    for (const auto& f : tmp_files) {
        if (f == "." || f == "..")
            continue;

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

    cout << "file listed: size=" << files.size() << endl;

    ACAnalyzer analyzer;

    int match_no = 0;
    bool game_end_detected = false;

    for (size_t i = 0; i < files.size(); ++i) {
        const auto& f = files[i];
        UniqueSDLSurface surface = makeUniqueSDLSurface(IMG_Load(f.c_str()));

        CaptureGameState state = analyzer.detectGameState(surface.get());
        if (state == CaptureGameState::LEVEL_SELECT) {
            // game start
            if (game_end_detected) {
                match_no += 1;
                game_end_detected = false;
                cout << "match found: " << match_no << " file="  << f << endl;

                char path[1024];
                sprintf(path, "out/%d", match_no);
                (void)mkdir(path, 0755);
            }
        } else if (isGameFinishedState(state)) {
            if (!game_end_detected) {
                cout << "match end: " << f << endl;
                game_end_detected = true;
            }
        }

#if 1
        // Will enable this later.
        // Copy image.
        char dest_path[1024];
        sprintf(dest_path, "out/%d/%s", match_no, file::basename(f).c_str());
        cout << dest_path << endl;

        PCHECK(symlink(f.c_str(), dest_path) == 0)
            << "target=" << f
            << " dest_path=" << dest_path;
#endif
#if 0
        CHECK(file::copyFile(f, dest_path))
            << "src=" << f
            << " dest=" << dest_path;
#endif
    }

    return 0;
}
