#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <signal.h>
#if OS_POSIX
#include <libgen.h>
#endif
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/file/path.h"
#include "core/server/connector/connector_manager.h"
#include "core/server/connector/human_connector.h"
#include "core/server/game_state.h"
#include "core/server/game_state_observer.h"
#include "duel/cui.h"
#if OS_WIN
#include "duel/cui_win.h"
#endif
#include "duel/duel_server.h"
#include "duel/puyofu_recorder.h"

#ifdef USE_HTTPD
#include "net/httpd/http_handler.h"
#include "net/httpd/http_server.h"
#endif

#ifdef USE_SDL2
#include <SDL.h>
#include "core/server/commentator.h"
#include "gui/commentator_drawer.h"
#include "gui/field_drawer.h"
#include "gui/fps_drawer.h"
#include "gui/human_connector_key_listener.h"
#include "gui/main_window.h"
#include "gui/user_event_drawer.h"
#endif

#if USE_AUDIO_COMMENTATOR
#include "audio/audio_commentator.h"
#include "audio/audio_server.h"
#include "internal/audio/internal_speaker.h"
#endif

using namespace std;

DEFINE_string(record, "", "use Puyofu Recorder. 'transition' for transition log, 'field' for field log");
DEFINE_bool(ignore_sigpipe, false, "true to ignore SIGPIPE");
#ifdef USE_HTTPD
DEFINE_bool(httpd, false, "use httpd");
DEFINE_int32(httpd_port, 8000, "httpd port");
DECLARE_string(data_dir);
#endif

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

class GameStateHandler : public GameStateObserver {
public:
    GameStateHandler() {}
    virtual ~GameStateHandler() {}

#if USE_HTTPD
    void handle(const HttpRequest& req, HttpResponse* resp) {
        UNUSED_VARIABLE(req);

        lock_guard<mutex> lock(mu_);
        if (!gameState_)
            return;
        resp->setContent(gameState_->toJson());
    }
#endif

    virtual void onUpdate(const GameState& gameState) override {
        lock_guard<mutex> lock(mu_);
        gameState_.reset(new GameState(gameState));
    }

private:
    mutex mu_;
    unique_ptr<GameState> gameState_;
};

#if !defined(_MSC_VER)
static void ignoreSIGPIPE()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));

    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);

    CHECK(sigaction(SIGPIPE, &act, 0) == 0);
}
#endif

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

#if !defined(_MSC_VER)
    if (FLAGS_ignore_sigpipe)
        ignoreSIGPIPE();
#endif

    if (argc != 3) {
        LOG(ERROR) << "There must be 2 arguments." << endl;
        return 1;
    }

#if USE_SDL2
    if (FLAGS_use_gui) {
        SDL_Init(SDL_INIT_VIDEO);
        atexit(SDL_Quit);
    }
#endif

    ConnectorManager manager(false);
    manager.setPlayer(0, argv[1]);
    manager.setPlayer(1, argv[2]);

    manager.start();

#ifdef USE_HTTPD
    unique_ptr<GameStateHandler> gameStateHandler;
    unique_ptr<HttpServer> httpServer;
    if (FLAGS_httpd) {
        gameStateHandler.reset(new GameStateHandler);
        httpServer.reset(new HttpServer(FLAGS_httpd_port));
        httpServer->installHandler("/data", [&](const HttpRequest& req, HttpResponse* res){
            gameStateHandler->handle(req, res);
        });
        httpServer->setAssetDirectory(file::joinPath(FLAGS_data_dir, "assets"));
    }
#endif

    unique_ptr<Cui> cui;
    if (FLAGS_use_cui) {
#if OS_WIN
        cui.reset(new CuiWin);
#else
        cui.reset(new Cui);
#endif
        cui->clear();
    }

    unique_ptr<PuyofuRecorder> puyofuRecorder;
    if (FLAGS_record == "transition") {
        puyofuRecorder.reset(new PuyofuRecorder);
        puyofuRecorder->setMode(PuyofuRecorder::Mode::TRANSITION_LOG);
    } else if (FLAGS_record == "field") {
        puyofuRecorder.reset(new PuyofuRecorder);
        puyofuRecorder->setMode(PuyofuRecorder::Mode::FIELD_LOG);
    } else if (FLAGS_record != "") {
        CHECK(false) << "Unknown --record value: " << FLAGS_record;
    }

#if USE_SDL2
    vector<unique_ptr<MainWindow::EventListener>> eventListeners;
    unique_ptr<MainWindow> mainWindow;
    unique_ptr<FieldDrawer> fieldDrawer;
    unique_ptr<Commentator> commentator;
    unique_ptr<CommentatorDrawer> commentatorDrawer;
    unique_ptr<FPSDrawer> fpsDrawer;
    unique_ptr<UserEventDrawer> userEventDrawer;
    if (FLAGS_use_gui) {
        if (FLAGS_use_commentator)
            mainWindow.reset(new MainWindow(640 + 2 * 144, 448 + 176, Box(144, 40, 144 + 640, 40 + 448)));
        else
            mainWindow.reset(new MainWindow(640, 448, Box(0, 0, 640, 448)));

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

        for (int i = 0; i < 2; ++i) {
            ServerConnector* c = manager.connector(i);
            if (c->isHuman()) {
                HumanConnector* hc = static_cast<HumanConnector*>(c);
                unique_ptr<MainWindow::EventListener> listener(new HumanConnectorKeyListener(hc));
                eventListeners.push_back(move(listener));
            }
        }

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

    DuelServer duelServer(&manager);

    // --- Add necessary obesrvers here.
#if USE_HTTPD
    if (gameStateHandler.get())
        duelServer.addObserver(gameStateHandler.get());
#endif
    if (cui.get())
        duelServer.addObserver(cui.get());
    if (puyofuRecorder.get())
        duelServer.addObserver(puyofuRecorder.get());
#if USE_SDL2
    if (fieldDrawer.get())
        duelServer.addObserver(fieldDrawer.get());
    if (commentator.get())
        duelServer.addObserver(commentator.get());
    if (userEventDrawer.get())
        duelServer.addObserver(userEventDrawer.get());
#endif
#if USE_HTTPD
    if (httpServer.get())
        CHECK(httpServer->start());
#endif
#if USE_AUDIO_COMMENTATOR
    if (audioCommentator.get())
        duelServer.addObserver(audioCommentator.get());
    if (audioServer.get())
        audioServer->start();
#endif

#if USE_SDL2
    auto duelSeverWillStopCallback = []() {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    };
    duelServer.setCallbackDuelServerWillExit(duelSeverWillStopCallback);
#endif
    CHECK(duelServer.start());

#if USE_SDL2
    if (commentator.get())
        CHECK(commentator->start());

    if (mainWindow.get()) {
        mainWindow->runMainLoop();
        duelServer.stop();
        manager.stop();
    }

    if (commentator.get())
        commentator->stop();
#endif

    duelServer.join();
#if USE_HTTPD
    if (httpServer.get())
        httpServer->stop();
#endif
#if USE_AUDIO_COMMENTATOR
    if (audioServer.get())
        audioServer->stop();
#endif

    return 0;
}
