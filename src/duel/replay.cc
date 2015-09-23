#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <signal.h>
#include <libgen.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/file.h"
#include "core/httpd/http_server.h"
#include "core/server/connector/human_connector.h"
#include "core/server/connector/connector_manager_posix.h"
#include "core/server/game_state.h"
#include "core/server/game_state_observer.h"
#include "duel/cui.h"
#include "duel/replay_server.h"
#include "duel/puyofu_recorder.h"

#ifdef USE_SDL2
#include <SDL.h>
#include "core/server/commentator.h"
#include "gui/commentator_drawer.h"
#include "gui/field_drawer.h"
#include "gui/fps_drawer.h"
#include "gui/human_connector_key_listener.h"
#include "gui/main_window.h"
#include "gui/player_ui.h"
#include "gui/user_event_drawer.h"
#endif

#if USE_AUDIO_COMMENTATOR
#include "audio/audio_commentator.h"
#include "audio/audio_server.h"
#include "internal/audio/internal_speaker.h"
#endif

using namespace std;

DEFINE_string(replay, "", "replay");
DEFINE_bool(ignore_sigpipe, false, "true to ignore SIGPIPE");
#ifdef USE_SDL2
DEFINE_bool(use_gui, true, "use GUI version drawer");
DEFINE_bool(use_commentator, true, "use commentator");
DEFINE_bool(use_cui, false, "use CUI version drawer");
#else
// When SDL2 is not defined, CUI should be enabled by default.
DEFINE_bool(use_cui, true, "use CUI version drawer");
#endif
#if USE_AUDIO_COMMENTATOR
DEFINE_bool(use_audio, false, "use audio commentator");
#endif

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
    google::InitGoogleLogging(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();

    if (FLAGS_ignore_sigpipe)
        ignoreSIGPIPE();

    if (argc != 4) {
        LOG(ERROR) << "There must be 4 argument." << endl;
        return 1;
    }

#if USE_SDL2
    if (FLAGS_use_gui) {
        SDL_Init(SDL_INIT_VIDEO);
        atexit(SDL_Quit);
    }
#endif

    ConnectorManagerPosix manager {
      Connector::create(0, string(argv[1])),
      Connector::create(1, string(argv[2])),
    };

    ifstream ifs(argv[3]);
    Json::Value root;
    Json::Reader reader;
    reader.parse(ifs, root);

    std::vector<GameState> states;
    for (int i = 0; i < int(root.size()); i++) {
      states.emplace_back(root[i]);
    }
    ReplayServer replayServer(&manager, states);
    
    unique_ptr<Cui> cui;
    if (FLAGS_use_cui) {
        cui.reset(new Cui);
        cui->clear();
    }

#if USE_SDL2
    vector<unique_ptr<MainWindow::EventListener>> eventListeners;
    unique_ptr<MainWindow> mainWindow;
    unique_ptr<FieldDrawer> fieldDrawer;
    unique_ptr<Commentator> commentator;
    unique_ptr<CommentatorDrawer> commentatorDrawer;
    unique_ptr<FPSDrawer> fpsDrawer;
    unique_ptr<UserEventDrawer> userEventDrawer;
    unique_ptr<PlayerUi> playerUi;
    if (FLAGS_use_gui) {
        if (FLAGS_use_commentator)
            mainWindow.reset(new MainWindow(640 + 2 * 144, 448 + 176 + 40, Box(144, 40, 144 + 640, 40 + 448)));
        else
            mainWindow.reset(new MainWindow(640, 448 + 40, Box(0, 0, 640, 448)));

        fieldDrawer.reset(new FieldDrawer);
        mainWindow->addDrawer(fieldDrawer.get());

        if (FLAGS_use_commentator) {
            commentator.reset(new Commentator);
            commentatorDrawer.reset(new CommentatorDrawer);
            commentator->addCommentatorObserver(commentatorDrawer.get());
            mainWindow->addDrawer(commentatorDrawer.get());
        }

        fpsDrawer.reset(new FPSDrawer);
        mainWindow->addDrawer(fpsDrawer.get());

        userEventDrawer.reset(new UserEventDrawer);
        mainWindow->addDrawer(userEventDrawer.get());

        playerUi.reset(new PlayerUi(&replayServer));
        mainWindow->addDrawer(playerUi.get());
        mainWindow->addEventListener(playerUi.get());

        for (auto& p : eventListeners) {
            mainWindow->addEventListener(p.get());
        }
    }
#endif

#if USE_AUDIO_COMMENTATOR
    unique_ptr<InternalSpeaker> internalSpeaker;
    unique_ptr<AudioServer> audioServer;
    unique_ptr<AudioCommentator> audioCommentator;

    if (FLAGS_use_commentator && FLAGS_use_gui && FLAGS_use_audio) {
        internalSpeaker.reset(new InternalSpeaker);
        audioServer.reset(new AudioServer(internalSpeaker.get()));
        audioCommentator.reset(new AudioCommentator(audioServer.get()));
        commentator->addCommentatorObserver(audioCommentator.get());
    }
#endif

    // --- Add necessary obesrvers here.
    if (cui.get())
      replayServer.addObserver(cui.get());
#if USE_SDL2
    if (fieldDrawer.get())
      replayServer.addObserver(fieldDrawer.get());
    if (commentator.get())
      replayServer.addObserver(commentator.get());
    if (userEventDrawer.get())
      replayServer.addObserver(userEventDrawer.get());
#endif
#if USE_AUDIO_COMMENTATOR
    if (audioCommentator.get())
      replayServer.addObserver(audioCommentator.get());
    if (audioServer.get())
      audioServer->start();
#endif

#if USE_SDL2
    auto duelSeverWillStopCallback = []() {
      SDL_Event event;
      event.type = SDL_QUIT;
      SDL_PushEvent(&event);
    };
    replayServer.setCallbackDuelServerWillExit(duelSeverWillStopCallback);
#endif
    CHECK(replayServer.start());

#if USE_SDL2
    if (commentator.get())
      CHECK(commentator->start());

    if (mainWindow.get()) {
      mainWindow->runMainLoop();
      replayServer.stop();
    }

    if (commentator.get())
      commentator->stop();
#endif

    replayServer.join();
#if USE_AUDIO_COMMENTATOR
    if (audioServer.get())
      audioServer->stop();
#endif

    return 0;
}
