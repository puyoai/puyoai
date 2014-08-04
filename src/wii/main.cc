#include <assert.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "capture/capture.h"
#include "capture/screen_shot_saver.h"
#include "capture/somagic_analyzer.h"
#include "capture/somagic_source.h"
#include "core/server/connector/connector_manager_linux.h"
#include "gui/main_window.h"
#include "wii/serial_key_sender.h"
#include "wii/stdout_key_sender.h"
#include "wii/wii_connect_server.h"

using namespace std;

DEFINE_bool(save_screenshot, false, "save screenshot");
DEFINE_bool(draw_result, true, "draw analyzer result");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    signal(SIGPIPE, SIG_IGN);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <serial> <1p> <2p>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    SomagicSource source("connect");
    SomagicAnalyzer analyzer;

    unique_ptr<KeySender> keySender;
    if (string(argv[1]) == "stdout") {
        keySender.reset(new StdoutKeySender);
    } else {
        keySender.reset(new SerialKeySender(argv[1]));
    }
    CHECK(source.ok());

    unique_ptr<ScreenShotSaver> saver;
    if (FLAGS_save_screenshot) {
        cout << "save screenshot: on" << endl;
        saver.reset(new ScreenShotSaver);
        saver->setDrawsFrameId(true);
    }

    WiiConnectServer server(&source, &analyzer, keySender.get(), argv[2], argv[3]);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&server));

    MainWindow mainWindow(720, 480, Box(0, 0, 720, 480));
    mainWindow.addDrawer(&server);
    if (saver.get())
        mainWindow.addDrawer(saver.get());
    if (analyzerResultDrawer.get())
        mainWindow.addDrawer(analyzerResultDrawer.get());

    source.start();
    server.start();

    mainWindow.runMainLoop();

    server.stop();

    return 0;
}
