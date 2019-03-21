#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <SDL.h>
#include <SDL_image.h>

#include "capture/capture.h"
#include "capture/color.h"
#include "gui/bounding_box.h"
#include "gui/main_window.h"
#include "gui/unique_sdl_surface.h"
#include "gui/SDL_prims.h"

#if defined(PUYOPUYO_TSU)
#include "capture/ac_analyzer.h"
#elif defined(PUYOPUYO_ESPORTS)
#include "capture/es_analyzer.h"
#endif

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <in-bmp>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(argv[1])));
    if (!surf) {
        fprintf(stderr, "Failed to load %s!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

#if defined(PUYOPUYO_TSU)
    ACAnalyzer analyzer;
#elif defined(PUYOPUYO_ESPORTS)
    ESAnalyzer analyzer;
#endif
    unique_ptr<AnalyzerResult> result(analyzer.analyze(surf.get(), nullptr, nullptr, nullptr, deque<unique_ptr<AnalyzerResult>>()));

    analyzer.drawWithAnalysisResult(surf.get());
    SDL_SaveBMP(surf.get(), "output.bmp");

    cout << result->toString() << endl;

    return 0;
}
