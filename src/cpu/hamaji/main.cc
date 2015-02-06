#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/client/ai/raw_ai.h"
#include "core/frame_request.h"
#include "core/frame_response.h"

#include "base.h"
#include "core.h"
#include "game.h"

DEFINE_bool(puyo_cloud_worker, false, "work as puyocloud worker");
DEFINE_bool(handle_opponent_grounded, true, "");

// TODO(mayah): g_core should be held in HamajiAI.
static Core* g_core;

class HamajiAI : public RawAI {
public:
  HamajiAI();
  ~HamajiAI() override {}
protected:
  FrameResponse playOneFrame(const FrameRequest&) override;
private:
  auto_ptr<Game> prev_game;
};

HamajiAI::HamajiAI() {
  prev_game.reset(new Game());
}

FrameResponse tick(Game* game) {
  if (game->p[1].state.grounded) {
    int chain_cnt;
    int score;
    int frame_cnt;

    game->p[1].f.SafeDrop();
    LF f(game->p[1].f);
    f.Simulate(&chain_cnt, &score, &frame_cnt);
    if (score) {
      score += game->p[1].score - game->p[1].spent_score;
      int ojama_cnt = score / 70;
      game->p[1].spent_score += ojama_cnt * 70;
      game->p[1].expected_ojama += ojama_cnt;
      game->p[1].expected_frame = frame_cnt;
#if 0
      emergency = FLAGS_handle_opponent_grounded;
#endif
      LOG(INFO) << "Opponent fired! ojama=" << game->p[1].expected_ojama
                << " frames=" << game->p[1].expected_frame
                << "\n" << game->p[1].f.GetDebugOutput();
    }
  }

#if 0
  // TODO(hamaji): Emergency handling was removed.
  //if ((g_should_decide && (state & STATE_YOU_CAN_PLAY)) || emergency) {
#endif
  if (game->p[0].state.decisionRequest || game->p[0].state.decisionRequestAgain) {
    Decision decision = g_core->decide(game);
    string message = g_core->msg();

#if 0
    // TODO(mayah): Support mawashi-area
    char buf[1024];
    int i = sprintf(buf, "ID=%d X=%d R=%d MSG=%-40s MA=",
                    game->id, decision.x, decision.r, g_core->msg().c_str());
    for (int y = 13; y <= 14; y++) {
      for (int x = 1; x <= 6; x++) {
        // TODO: Not good to use ordinal(). Define toDeprecatedChar() in core?
        char c = ordinal(game->p[0].f.Get(x, y)) + '0';
        buf[i++] = c;
      }
    }
    buf[i] = 0;
#endif

    g_core->clear_msg();
    if (decision.isValid()) {
      game->decided_field = game->p[0].f;
      game->decided_field.PutDecision(decision,
                                      game->p[0].next.axis(0),
                                      game->p[0].next.child(1));
    }
#if 0
    LOG(INFO) << "<= " << buf
              << " (cnt=" << game->p[0].f.countPuyo() << ")";
#endif
    return FrameResponse(game->id, decision, message);
  }

  return FrameResponse(game->id);
}

FrameResponse HamajiAI::playOneFrame(const FrameRequest& request) {
  if (request.gameResult != GameResult::PLAYING) {
    Game::reset();
    if (request.gameResult == GameResult::P2_WIN || request.gameResult == GameResult::P2_WIN_WITH_CONNECTION_ERROR) {
      LOG(INFO) << "=== YOU /(^o^)\\ LOSE ===";
    } else if (request.gameResult == GameResult::DRAW) {
      LOG(INFO) << "=== DRAW -(-_-)- GAME ===";
    } else if (request.gameResult == GameResult::P1_WIN || request.gameResult == GameResult::P1_WIN_WITH_CONNECTION_ERROR) {
      LOG(INFO) << "=== YOU \\(^o^)/ WIN ===";
    }
    prev_game.reset(new Game());

    return FrameResponse(request.frameId);
  }

  auto_ptr<Game> game(new Game(*prev_game, request));

  FrameResponse response = tick(game.get());
  prev_game = game;
  return response;
}

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  g_core = new Core(false);

  HamajiAI().runLoop();
  return 0;
}
