#include <assert.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "capture/ac_analyzer.h"
#include "capture/capture.h"
#include "capture/screen_shot_saver.h"
#include "capture/somagic_source.h"
#include "capture/syntek_source.h"
#include "capture/movie_source.h"
#include "capture/movie_source_key_listener.h"
#include "core/server/commentator.h"
#include "gui/commentator_drawer.h"
#include "gui/decision_drawer.h"
#include "gui/main_window.h"
#include "gui/user_event_drawer.h"
#include "wii/serial_key_sender.h"
#include "wii/stdout_key_sender.h"
#include "wii/wii_connect_server.h"

#if USE_AUDIO_COMMENTATOR
#include "audio/audio_commentator.h"
#include "audio/audio_server.h"
#include "internal/audio/internal_speaker.h"
#endif

using namespace std;

DEFINE_bool(save_screenshot, false, "save screenshot");
DEFINE_bool(draw_result, true, "draw analyzer result");
DEFINE_bool(draw_decision, true, "draw decision");
DEFINE_string(source, "somagic",
              "set image source. 'somagic' when using somagic video capture."
              " filename if you'd like to use movie.");
DEFINE_int32(fps, 30, "FPS");
DEFINE_bool(ignore_sigpipe, false, "ignore SIGPIPE");
DEFINE_bool(use_commentator, false, "use commentator");

#if USE_AUDIO_COMMENTATOR
DEFINE_bool(use_audio, false, "use audio commentator");
#endif

static unique_ptr<Source> makeVideoSource()
{
    if (FLAGS_source == "somagic")
        return unique_ptr<Source>(new SomagicSource("connect"));
    if (FLAGS_source == "syntek")
        return unique_ptr<Source>(new SyntekSource);

    MovieSource* source = new MovieSource(FLAGS_source);
    CHECK(source->ok());
    source->setFPS(FLAGS_fps);

    return unique_ptr<Source>(source);
}

static unique_ptr<Analyzer> makeVideoAnalyzer()
{
    return unique_ptr<Analyzer>(new ACAnalyzer);
}

static void ignoreSIGPIPE()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));

    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);

    CHECK(sigaction(SIGPIPE, &act, 0) == 0);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (FLAGS_ignore_sigpipe)
        ignoreSIGPIPE();

    if (argc < 4) {
        fprintf(stderr, "Usage: %s [option] <serial> <1p> <2p>\n", argv[0]);
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource::init();

    unique_ptr<Source> source = makeVideoSource();
    unique_ptr<Analyzer> analyzer = makeVideoAnalyzer();

    unique_ptr<KeySender> keySender;
    if (string(argv[1]) == "stdout") {
        keySender.reset(new StdoutKeySender);
    } else {
        keySender.reset(new SerialKeySender(argv[1]));
    }
    CHECK(source->ok());

    unique_ptr<ScreenShotSaver> saver;
    if (FLAGS_save_screenshot) {
        cout << "save screenshot: on" << endl;
        saver.reset(new ScreenShotSaver);
        saver->setDrawsFrameId(true);
    }

    WiiConnectServer server(source.get(), analyzer.get(), keySender.get(), argv[2], argv[3]);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&server));

    unique_ptr<DecisionDrawer> decisionDrawer;
    if (FLAGS_draw_decision)
        decisionDrawer.reset(new DecisionDrawer);

    unique_ptr<UserEventDrawer> userEventDrawer;
    userEventDrawer.reset(new UserEventDrawer);

    unique_ptr<MovieSourceKeyListener> movieSourceKeyListener;
    // TODO(mayah): BAD! Don't check FLAGS_source here.
    if (FLAGS_fps == 0 && FLAGS_source != "somagic" && FLAGS_source != "syntek") {
        MovieSource* movieSource = static_cast<MovieSource*>(source.get());
        movieSourceKeyListener.reset(new MovieSourceKeyListener(movieSource));
    }

    unique_ptr<MainWindow> mainWindow;
    unique_ptr<Commentator> commentator;
    unique_ptr<CommentatorDrawer> commentatorDrawer;

    if (FLAGS_use_commentator) {
        mainWindow.reset(new MainWindow(720 + 2 * 144, 480 + 176, Box(144, 40, 144 + 720, 40 + 480)));
    } else {
        mainWindow.reset(new MainWindow(720, 480, Box(0, 0, 720, 480)));
    }

    mainWindow->addDrawer(&server);
    if (saver.get())
        mainWindow->addDrawer(saver.get());
    if (analyzerResultDrawer.get())
        mainWindow->addDrawer(analyzerResultDrawer.get());
    if (movieSourceKeyListener.get())
        mainWindow->addEventListener(movieSourceKeyListener.get());

    if (FLAGS_use_commentator) {
        commentator.reset(new Commentator);
        commentatorDrawer.reset(new CommentatorDrawer);
        commentator->addCommentatorObserver(commentatorDrawer.get());
        mainWindow->addDrawer(commentatorDrawer.get());
    }

    if (decisionDrawer.get())
        mainWindow->addDrawer(decisionDrawer.get());
    if (userEventDrawer.get())
        mainWindow->addDrawer(userEventDrawer.get());

#if USE_AUDIO_COMMENTATOR
    unique_ptr<InternalSpeaker> internalSpeaker;
    unique_ptr<AudioServer> audioServer;
    unique_ptr<AudioCommentator> audioCommentator;

    if (FLAGS_use_commentator && FLAGS_use_audio) {
        internalSpeaker.reset(new InternalSpeaker);
        audioServer.reset(new AudioServer(internalSpeaker.get()));
        audioCommentator.reset(new AudioCommentator(audioServer.get()));
        commentator->addCommentatorObserver(audioCommentator.get());
    }
#endif

    if (commentator.get())
        server.addObserver(commentator.get());
    if (decisionDrawer.get())
        server.addObserver(decisionDrawer.get());
    if (userEventDrawer.get())
        server.addObserver(userEventDrawer.get());
#if USE_AUDIO_COMMENTATOR
    if (audioCommentator.get())
        server.addObserver(audioCommentator.get());
    if (audioServer.get())
        audioServer->start();
#endif

    source->start();
    server.start();
    if (commentator.get())
        CHECK(commentator->start());

    mainWindow->runMainLoop();

    server.stop();
    if (commentator.get())
        commentator->stop();

#if USE_AUDIO_COMMENTATOR
    if (audioServer.get())
        audioServer->stop();
#endif

    return 0;
}
