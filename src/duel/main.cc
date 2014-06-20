#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <libgen.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/server/connector/connector_manager_linux.h"
#include "duel/cui.h"
#include "duel/duel_server.h"
#include "duel/http_server.h"
#include "duel/game_state.h"
#include "duel/game_state_observer.h"
#include "duel/puyofu_recorder.h"

#ifdef USE_SDL2
#include <SDL.h>
#include "gui/commentator.h"
#include "gui/field_drawer.h"
#include "gui/fps_drawer.h"
#include "gui/main_window.h"
#endif

using namespace std;

DEFINE_bool(record, false, "use Puyofu Recorder");

#ifdef USE_SDL2
DEFINE_bool(use_gui, true, "use GUI version drawer");
DEFINE_bool(use_commentator, true, "use commentator");
DEFINE_bool(use_cui, false, "use CUI version drawer");
#else
// When SDL2 is not defined, CUI should be enabled by default.
DEFINE_bool(use_cui, true, "use CUI version drawer");
#endif

class GameStateHandler : public HttpHandler, public GameStateObserver {
public:
    GameStateHandler() {}
    virtual ~GameStateHandler() {}

    virtual void handle(HttpRequest* req, HttpResponse* resp) OVERRIDE {
        lock_guard<mutex> lock(mu_);
        UNUSED_VARIABLE(req);
        if (!gameState_)
            return;
        resp->setContent(gameState_->toJson());
    }

    virtual void onUpdate(const GameState& gameState) OVERRIDE {
        lock_guard<mutex> lock(mu_);
        gameState_.reset(new GameState(gameState));
    }

private:
    mutex mu_;
    unique_ptr<GameState> gameState_;
};

static string getExecDirName(const char* argv0)
{
    unique_ptr<char, void (*)(void*)> x(strdup(argv0), std::free);
    if (!x)
        return string(".");

    return string(dirname(x.get()));
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();

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

    string dir = getExecDirName(argv[0]);
    ConnectorManagerLinux manager {
        Connector::create(0, string(argv[1])),
        Connector::create(1, string(argv[2])),
    };

#if USE_HTTPD
    GameStateHandler gameStateHandler;
    HttpServer httpServer(dir);
    httpServer.installHandler("/data", &gameStateHandler);
#endif

    unique_ptr<Cui> cui;
    if (FLAGS_use_cui) {
        cui.reset(new Cui);
        cui->clear();
    }

    unique_ptr<PuyofuRecorder> puyofuRecorder;
    if (FLAGS_record)
        puyofuRecorder.reset(new PuyofuRecorder);

#if USE_SDL2
    unique_ptr<MainWindow> mainWindow;
    unique_ptr<FieldDrawer> fieldDrawer;
    unique_ptr<Commentator> commentator;
    unique_ptr<FPSDrawer> fpsDrawer;
    if (FLAGS_use_gui) {
        if (FLAGS_use_commentator)
            mainWindow.reset(new MainWindow(800, 600, Box(144, 40, 656, 424)));
        else
            mainWindow.reset(new MainWindow(512, 384, Box(0, 0, 512, 384)));

        fieldDrawer.reset(new FieldDrawer);
        mainWindow->addDrawer(fieldDrawer.get());

        if (FLAGS_use_commentator) {
            commentator.reset(new Commentator);
            mainWindow->addDrawer(commentator.get());
        }

        fpsDrawer.reset(new FPSDrawer);
        mainWindow->addDrawer(fpsDrawer.get());
    }
#endif

    DuelServer duelServer(&manager);

    // --- Add necessary obesrvers here.
#if USE_HTTPD
    duelServer.addObserver(&gameStateHandler);
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
#endif

#if USE_HTTPD
    CHECK(httpServer.start());
#endif
    CHECK(duelServer.start());

#if USE_SDL2
    if (commentator.get())
        CHECK(commentator->start());

    if (mainWindow.get()) {
        mainWindow->runMainLoop();
        duelServer.stop();
    }

    if (commentator.get())
        commentator->stop();
#endif

    duelServer.join();
#if USE_HTTPD
    httpServer.stop();
#endif

    return 0;
}
