#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>

#include "base/file/path.h"
#include "base/strings.h"
#include "capture/ac_analyzer.h"
#include "gui/unique_sdl_surface.h"

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
            }
        } else if (isGameFinishedState(state)) {
            game_end_detected = true;
        }

#if 0
        // Will enable this later.

        // Copy image.
        char dest_path[1024];
        sprintf(dest_path, "out/%d/%s", match_no, file::basename(f).c_str());
        CHECK(file::copyFile(f, dest_path))
            << "src=" << f
            << " dest=" << dest_path;
#endif
    }

    CHECK(false) << "Not implemented yet";
    return 0;
}
