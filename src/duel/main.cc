#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <libgen.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/server/connector/human_connector.h"
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
#include "gui/human_connector_key_listener.h"
#include "gui/main_window.h"
#endif

using namespace std;

DEFINE_bool(record, false, "use Puyofu Recorder");
#ifdef USE_HTTPD
DEFINE_bool(httpd, false, "use httpd");
#endif

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

    ConnectorManagerLinux manager {
        Connector::create(0, string(argv[1])),
        Connector::create(1, string(argv[2])),
    };

#ifdef USE_HTTPD
    unique_ptr<GameStateHandler> gameStateHandler;
    unique_ptr<HttpServer> httpServer;
    if (FLAGS_httpd) {
        gameStateHandler.reset(new GameStateHandler);
        httpServer.reset(new HttpServer);
        httpServer->installHandler("/data", gameStateHandler.get());
    }
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
    vector<unique_ptr<MainWindow::EventListener>> eventListeners;
    unique_ptr<MainWindow> mainWindow;
    unique_ptr<FieldDrawer> fieldDrawer;
    unique_ptr<Commentator> commentator;
    unique_ptr<FPSDrawer> fpsDrawer;
    if (FLAGS_use_gui) {
        if (FLAGS_use_commentator)
            mainWindow.reset(new MainWindow(640 + 2 * 144, 490 + 176, Box(144, 40, 144 + 640, 40 + 490)));
        else
            mainWindow.reset(new MainWindow(640, 490, Box(0, 0, 640, 490)));

        fieldDrawer.reset(new FieldDrawer);
        mainWindow->addDrawer(fieldDrawer.get());

        if (FLAGS_use_commentator) {
            commentator.reset(new Commentator);
            mainWindow->addDrawer(commentator.get());
        }

        fpsDrawer.reset(new FPSDrawer);
        mainWindow->addDrawer(fpsDrawer.get());

        for (int i = 0; i < 2; ++i) {
            Connector* c = manager.connector(i);
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
#endif

#if USE_HTTPD
    if (httpServer.get())
        CHECK(httpServer->start());
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
    if (httpServer.get())
        httpServer->stop();
#endif

    return 0;
}
