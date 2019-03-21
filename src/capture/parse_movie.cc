#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "capture/capture.h"
#include "capture/monitor.h"
#include "capture/movie_source.h"
#include "capture/movie_source_key_listener.h"
#include "gui/bounding_box_drawer.h"
#include "gui/box.h"
#include "gui/main_window.h"
#include "gui/screen.h"

#if defined(PUYOPUYO_TSU)
#include "capture/ac_analyzer.h"
#elif defined(PUYOPUYO_ESPORTS)
#include "capture/es_analyzer.h"
#endif

#ifdef USE_V4L2
# include "capture/viddev_source.h"
#endif


DEFINE_bool(draw_result, true, "draw analyzer result");
DEFINE_int32(fps, 60, "FPS. When 0, hitting space will go next step.");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <in-movie>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    MovieSource::init();

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource source(argv[1]);
    if (!source.ok()) {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    source.setFPS(FLAGS_fps);

#if defined(PUYOPUYO_TSU)
    ACAnalyzer analyzer;
#elif defined(PUYOPUYO_ESPORTS)
    ESAnalyzer analyzer;
#endif
    Capture capture(&source, &analyzer);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&capture));

    unique_ptr<MovieSourceKeyListener> movieSourceKeyListener;
    if (FLAGS_fps == 0) {
        movieSourceKeyListener.reset(new MovieSourceKeyListener(&source));
    }

    MainWindow mainWindow(kMonitorWidth, kMonitorHeight,
                          Box(0, 0, kMonitorWidth, kMonitorHeight));
    mainWindow.addDrawer(&capture);
    if (analyzerResultDrawer.get())
        mainWindow.addDrawer(analyzerResultDrawer.get());
    if (movieSourceKeyListener.get())
        mainWindow.addEventListener(movieSourceKeyListener.get());

    capture.start();

    mainWindow.runMainLoop();

    return 0;
}
