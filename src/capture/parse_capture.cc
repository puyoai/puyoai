#include <stdint.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/strings.h"
#include "capture/ac_analyzer.h"
#include "capture/capture.h"
#include "capture/syntek_source.h"
#include "gui/bounding_box_drawer.h"
#include "gui/box.h"
#include "gui/fps_drawer.h"
#include "gui/main_window.h"
#include "gui/screen.h"

#ifdef USE_V4L2
#include "capture/viddev_source.h"
#endif

#include <iostream>

DEFINE_bool(save_screenshot, false, "save screenshot");
DEFINE_bool(draw_result, true, "draw analyzer result");
DEFINE_string(source, "syntek", "set image source");

static unique_ptr<Source> makeVideoSource()
{
    if (FLAGS_source == "syntek")
        return unique_ptr<Source>(new SyntekSource);

#if USE_V4L2
    if (strings::isPrefix(FLAGS_source, "v4l2:")) {
        std::string deviceName = FLAGS_source.substr(5);
        cout << "V4L2 device name: " << deviceName << endl;
        return unique_ptr<Source>(new VidDevSource(deviceName));
    }
#endif

    return unique_ptr<Source>(nullptr);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    unique_ptr<Source> source(makeVideoSource());
    if (!source.get()) {
        printf("source is not initialized.\n");
        return 1;
    }

    ACAnalyzer analyzer;
    Capture capture(source.get(), &analyzer);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&capture));

    if (FLAGS_save_screenshot)
        source->setSavesScreenShot(true);

    unique_ptr<FPSDrawer> fpsDrawer(new FPSDrawer);

    MainWindow mainWindow(640, 448, Box(0, 0, 640, 448));
    mainWindow.addDrawer(&capture);
    if (analyzerResultDrawer.get())
        mainWindow.addDrawer(analyzerResultDrawer.get());
    if (fpsDrawer.get())
        mainWindow.addDrawer(fpsDrawer.get());

    source->start();
    capture.start();

    mainWindow.runMainLoop();

    return 0;
}
