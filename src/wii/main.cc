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
#include "capture/syntek_source.h"
#include "capture/movie_source.h"
#include "capture/movie_source_key_listener.h"
#include "core/server/commentator.h"
#include "core/server/game_state_recorder.h"
#include "gui/commentator_drawer.h"
#include "gui/decision_drawer.h"
#include "gui/frame_number_drawer.h"
#include "gui/main_window.h"
#include "gui/user_event_drawer.h"
#include "wii/serial_key_sender.h"
#include "wii/stdout_key_sender.h"
#include "wii/null_key_sender.h"
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
DEFINE_bool(draw_user_event, true, "draw user event");
DEFINE_bool(draw_frame_number, true, "draw frame number");
DEFINE_string(source, "syntek",
              "set image source. 'syntek' when using syntek video capture."
              " filename if you'd like to use movie.");
DEFINE_int32(fps, 60, "FPS");
DEFINE_bool(ignore_sigpipe, false, "ignore SIGPIPE");
DEFINE_bool(use_commentator, false, "use commentator");
DEFINE_bool(use_game_state_recorder, true, "use game state recorder");
DEFINE_string(record_dir, ".", "directory where game state is recorded");

#if USE_AUDIO_COMMENTATOR
DEFINE_bool(use_audio, false, "use audio commentator");
#endif

static unique_ptr<Source> makeVideoSource()
{
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
        fprintf(stderr, "       %s [option] <serial1p> <serial2p> <1p> <2p>\n", argv[0]);
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    MovieSource::init();

    unique_ptr<Source> source = makeVideoSource();
    unique_ptr<Analyzer> analyzer = makeVideoAnalyzer();

    unique_ptr<KeySender> keySender[2];
    if (string(argv[1]) == "stdout") {
        keySender[0].reset(new StdoutKeySender);
    } else if (string(argv[1]) == "-") {
        keySender[0].reset(new NullKeySender);
    } else {
        keySender[0].reset(new SerialKeySender(argv[1]));
    }
    if (string(argv[2]) == "stdout") {
        keySender[1].reset(new StdoutKeySender);
    } else if (string(argv[2]) == "-") {
        keySender[1].reset(new NullKeySender);
    } else {
        keySender[1].reset(new SerialKeySender(argv[2]));
    }

    CHECK(source->ok());

    if (FLAGS_save_screenshot) {
        source->setSavesScreenShot(true);
        cout << "save screenshot: on" << endl;
    }

    WiiConnectServer server(source.get(), analyzer.get(),
                            keySender[0].get(), keySender[1].get(),
                            argv[3], argv[4]);

    unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
    if (FLAGS_draw_result)
        analyzerResultDrawer.reset(new AnalyzerResultDrawer(&server));

    unique_ptr<DecisionDrawer> decisionDrawer;
    if (FLAGS_draw_decision)
        decisionDrawer.reset(new DecisionDrawer);

    unique_ptr<UserEventDrawer> userEventDrawer;
    if (FLAGS_draw_user_event)
        userEventDrawer.reset(new UserEventDrawer);

    unique_ptr<FrameNumberDrawer> frameNumberDrawer;
    if (FLAGS_draw_frame_number)
        frameNumberDrawer.reset(new FrameNumberDrawer);

    unique_ptr<MovieSourceKeyListener> movieSourceKeyListener;
    // TODO(mayah): BAD! Don't check FLAGS_source here.
    if (FLAGS_fps == 0 && FLAGS_source != "syntek") {
        MovieSource* movieSource = static_cast<MovieSource*>(source.get());
        movieSourceKeyListener.reset(new MovieSourceKeyListener(movieSource));
    }

    unique_ptr<MainWindow> mainWindow;
    unique_ptr<Commentator> commentator;
    unique_ptr<CommentatorDrawer> commentatorDrawer;

    if (FLAGS_use_commentator) {
        mainWindow.reset(new MainWindow(640 + 2 * 144, 448 + 176, Box(144, 40, 144 + 640, 40 + 448)));
    } else {
        mainWindow.reset(new MainWindow(640, 448, Box(0, 0, 640, 448)));
    }

    mainWindow->addDrawer(&server);
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
    if (frameNumberDrawer.get())
        mainWindow->addDrawer(frameNumberDrawer.get());

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

    unique_ptr<GameStateRecorder> gameStateRecorder;
    if (FLAGS_use_game_state_recorder) {
        gameStateRecorder.reset(new GameStateRecorder(FLAGS_record_dir));
    }
    if (gameStateRecorder.get())
        server.addObserver(gameStateRecorder.get());

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
