#include "movie.h"
#include "capture.h"
#include "images.h"
#include "viddev.h"

#include <core/ctrl.h>
#include <core/field.h>

#include <duel/connector_manager_linux.h>

#include <assert.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

using namespace std;

int g_output_keys_state = 0;
Field g_field[2];
Decision g_last_decision[2];
float g_estimated_key_sec[2];
static const float CAP_SEC = 1.0 / 60;
static const float HID_SEC = 1.0 / 40;
char g_invisible_puyos[2][7];
char g_cur_puyo1[2], g_cur_puyo2[2];

void resetGame() {
  memset(g_invisible_puyos, 0, sizeof(g_invisible_puyos));
  memset(g_cur_puyo1, 0, sizeof(g_cur_puyo1));
  memset(g_cur_puyo2, 0, sizeof(g_cur_puyo2));
}

void putPuyoToField(int pi, Field* f, Decision d, char c1, char c2) {
  int x1 = d.x;
  int x2 = d.x + (d.r == 1) - (d.r == 3);

  int heights[Field::MAP_WIDTH+1];
  for (int x = 1; x <= Field::WIDTH; x++) {
    heights[x] = 100;
    if (x1 != x && x2 != x)
      continue;
    for (int y = 1; y <= Field::HEIGHT + 2; y++) {
      if (f->Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  if (d.r == 2) {
    f->Set(x2, heights[x2]++, c2);
    f->Set(x1, heights[x1]++, c1);
  } else {
    f->Set(x1, heights[x1]++, c1);
    f->Set(x2, heights[x2]++, c2);
  }

  f->Simulate();

  for (int x = 0; x < 6; x++) {
    g_invisible_puyos[pi][x] = f->Get(x + 1, 13) + '0';
  }
}

void maybeOutputBanner(FILE* fp) {
  if (g_output_keys_state == 1) {
    fprintf(fp, "=== outputKeys start ===\n");
    g_output_keys_state = 2;
  }
}

void sendKey(Key k) {
  cout << k << endl;
  FILE* fp = fopen("/tmp/key.log", "ab");
  maybeOutputBanner(fp);
  fprintf(fp, "%d\n", k);
  fclose(fp);
}

void sendKey2(Key k1, Key k2) {
  cout << k1 << ' ' << k2 << endl;
  FILE* fp = fopen("/tmp/key.log", "ab");
  maybeOutputBanner(fp);
  fprintf(fp, "%d %d\n", k1, k2);
  fclose(fp);
}

void outputKeysImpl(int pi, const PlayerLog& data) {
  // Try all commands from the newest one.
  // If we find a command we can use, we'll ignore older ones.
  for (unsigned int i = data.received_data.size(); i > 0; ) {
    i--;

    // We don't need ACK/NACK for ID only messages.
    if (data.received_data[i].decision.x == 0 &&
        data.received_data[i].decision.r == 0) {
      continue;
    }

    Decision d = Decision(
        data.received_data[i].decision.x,
        data.received_data[i].decision.r);

    fprintf(stderr, "decision: x=%d r=%d (prev: x=%d r=%d)\n",
            d.x, d.r, g_last_decision[pi].x, g_last_decision[pi].r);
    FILE* fp = fopen("/tmp/key.log", "ab");
    fprintf(fp, "decision: x=%d r=%d (prev: x=%d r=%d)\n",
            d.x, d.r, g_last_decision[pi].x, g_last_decision[pi].r);
    fclose(fp);

    if (d.IsValid() && d != g_last_decision[pi]) {
      vector<KeyTuple> keys;
      KumipuyoPos start;
      if (g_last_decision[pi].IsValid()) {
        start.x = g_last_decision[pi].x;
        // TODO(hamaji): Set an appropriate value for Y.
        start.y = 12;
        start.r = g_last_decision[pi].r;
      } else {
        start.x = 3;
        start.y = 12;
        start.r = 0;
      }
      KumipuyoPos goal;
      goal.x = d.x;
      // It's OK as Ctrl::getControlOnline doesn't use Y.
      goal.y = 1;
      goal.r = d.r;

      Field f(g_field[pi]);
      // TODO(hamaji): Handle missing MA properly.
      const string& ma = data.received_data[i].mawashi_area;
      CHECK(ma.size() == 0 || ma.size() == 12)
        << "Invalid MA field! size=" << ma.size() << " ma=" << ma;
      for (size_t j = 0; j < ma.size(); j++) {
        int x = j % 6;
        int y = j / 6;
        char c = ma[j] - '0';
        CHECK_GE(c, 0) << "Invalid MA field! c=" << (int)c << " ma=" << ma;
        CHECK_LE(c, 7) << "Invalid MA field! c=" << (int)c << " ma=" << ma;
        f.Set(x, y, c);
      }

      if (Ctrl::getControlOnline(f, goal, start, &keys)) {
        if (g_cur_puyo1[pi] && g_cur_puyo2[pi]) {
          putPuyoToField(pi, &f, d,
                         g_cur_puyo1[pi], g_cur_puyo2[pi]);
        }
        g_last_decision[pi] = d;
        for (size_t j = 0; j < keys.size(); j++) {
          KeyTuple k = keys[j];
          // Slow mode
          //if (k.b1 == KEY_DOWN || k.b2 == KEY_DOWN)
          //  continue;
          if (k.b2 != KEY_NONE) {
            fprintf(stderr, "%d %d\n", k.b1, k.b2);
            sendKey2(k.b1, k.b2);
          } else {
            fprintf(stderr, "%d\n", k.b1);
            sendKey(k.b1);
          }

          if (j < keys.size() - 1 && k.hasSameKey(keys[j+1])) {
            sendKey(KEY_NONE);
          }
        }
        // TODO(hamaji): No idea about this 24
        g_estimated_key_sec[pi] += HID_SEC * keys.size() + CAP_SEC * 24;
        return;
      }
    } else if (d.x == -1 && d.r == -1) {
      return;
    }
  }
}

void outputKeys(int pi, const PlayerLog& data) {
  g_output_keys_state = 1;

  outputKeysImpl(pi, data);

  if (g_output_keys_state == 2) {
    FILE* fp = fopen("/tmp/key.log", "ab");
    fprintf(fp, "=== outputKeys end ===\n");
    fclose(fp);
  }
  g_output_keys_state = 0;
}

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  signal(SIGPIPE, SIG_IGN);

  if (argc < 4) {
    fprintf(stderr, "Usage: %s <source> <1p> <2p>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  SDL_Init(SDL_INIT_VIDEO);

  Source* source = NULL;
  int bpp = 0;
  if (!strcmp(argv[1], "images")) {
    vector<string> images(argv + 4, argv + argc);
    source = new Images(images);
    bpp = 16;
  } else if (strncmp(argv[1], "/dev", 4)) {
    Movie::init();
    source = new Movie(argv[1]);
    bpp = 24;
  }
#ifdef USE_V4L2
  else {
    source = new VidDev(argv[1]);
    bpp = 16;
  }
#endif
  CHECK(source);

  vector<string> program_names;
  bool is_ai[2];
  for (int i = 0; i < 2; i++) {
    string prog = argv[2 + i];
    program_names.push_back(prog);
    is_ai[i] = (prog != "-");
  }
  ConnectorManagerLinux* connector = new ConnectorManagerLinux(program_names);
  connector->DontWaitTimeout();

  if (!source->ok()) {
    fprintf(stderr, "Failed to load %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  Capture cap(source->width(), source->height(), bpp);
  SDL_Surface* scr = cap.getScreen()->scr();
  assert(scr);

  Uint32 start_ticks = SDL_GetTicks();

  int current_id = 0;
  Capture::GameState prev_game_state = Capture::GAME_INIT;
  while (!source->done()) {
    SDL_Surface* surf = source->getNextFrame();
    if (!surf) {
      fprintf(stderr, "end of the stream\n");
      break;
    }

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) {
        source->end();
      }
      if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_p) {
          Source::saveScreenShot(surf, scr);
          continue;
        } else if (ev.key.keysym.sym == SDLK_m) {
          fprintf(stderr, "%s\n", cap.getMessageFor(0).c_str());
          continue;
        } else if (ev.key.keysym.sym == SDLK_ESCAPE) {
          source->end();
          continue;
        }
      }

      source->handleEvent(ev);
    }
    source->handleKeys();

    if (Images* images = dynamic_cast<Images*>(source)) {
      if (images->isStopped() && !images->isIndexUpdated()) {
        SDL_Delay(10);
        continue;
      }
    }

    cap.addFrame(surf);

    if (cap.game_state() != prev_game_state) {
      FILE* fp = fopen("/tmp/key.log", "ab");
      fprintf(fp, "game state change: %d => %d\n",
              prev_game_state, cap.game_state());
      fclose(fp);
      sendKey(KEY_NONE);
      prev_game_state = cap.game_state();
    }

    for (int pi = 0; pi < 2; pi++) {
      for (int x = 0; x < 6; x++) {
        for (int y = 0; y < 12; y++) {
          g_field[pi].Set(x+1, 12-y, cap.getColor(pi, x, y));
        }
        g_field[pi].Set(x+1, 13, EMPTY);
        g_field[pi].Set(x+1, 14, EMPTY);
      }
      g_cur_puyo1[pi] = cap.getNext(pi, 1);
      g_cur_puyo2[pi] = cap.getNext(pi, 0);

      float s = g_estimated_key_sec[pi];
      g_estimated_key_sec[pi] = s - CAP_SEC;
      if (s > 0.0 && g_estimated_key_sec[pi] <= 0) {
        sendKey(KEY_NONE);
      }
    }

    if (cap.game_state() == Capture::GAME_RUNNING) {
      for (int pi = 0; pi < 2; pi++) {
        if (!connector->IsConnectorAlive(pi)) {
          fprintf(stderr, "player #%d was disconnected\n", pi);
          source->end();
          break;
        }

        if (is_ai[pi]) {
          string msg = cap.getMessageFor(pi);
#if 0
          size_t found = msg.find("YF=");
          assert(found != string::npos);
          msg = (msg.substr(0, found + 3) +
                 g_invisible_puyos[pi] + msg.substr(found + 3));
          //fprintf(stderr, "MOD: %s (%s)\n",
          //        msg.c_str(), g_invisible_puyos[pi]);
#endif
          connector->Write(pi, msg);
        }
      }

      vector<PlayerLog> all_data;
      connector->GetActions(current_id, &all_data);

      for (int pi = 0; pi < 2; pi++) {
        if (!is_ai[pi]) {
          continue;
        }

        if ((cap.getState(pi) & STATE_YOU_GROUNDED) == STATE_YOU_GROUNDED) {
          g_last_decision[pi] = Decision();
          fprintf(stderr, "turn end!\n");
          FILE* fp = fopen("/tmp/key.log", "ab");
          fprintf(fp, "GROUND(%d) ", pi);
          fclose(fp);
          sendKey(KEY_NONE);
          g_estimated_key_sec[pi] = 0.0;
        }

        const PlayerLog& data = all_data[pi];
        outputKeys(pi, data);

        for (unsigned int i = data.received_data.size(); i > 0; ) {
          i--;
          const string& msg = data.received_data[i].msg;
          if (!msg.empty()) {
            cap.setAIMessage(pi, msg);
            break;
          }
        }
      }
    } else if (cap.game_state() == Capture::GAME_MODE_SELECT) {
      if (current_id % 10 == 0) {
        sendKey(KEY_NONE);
        sendKey(KEY_RIGHT_TURN);
        sendKey(KEY_NONE);
      }
      resetGame();
    } else if (cap.game_state() == Capture::GAME_TITLE) {
      if (current_id % 10 == 0) {
        sendKey(KEY_NONE);
      }
      resetGame();
    } else if (cap.game_state() == Capture::GAME_FINISHED) {
      if (current_id % 10 == 0) {
        sendKey(KEY_NONE);
        sendKey((Key)(KEY_LEFT_TURN + 1));
        sendKey(KEY_NONE);
      }
      resetGame();
    }

    cap.show();

    current_id++;
    Uint32 now = SDL_GetTicks();
    if (0) {
      if (now != start_ticks) {
        fprintf(stderr, "%f fps\n",
                (float)current_id * 1000 / (now - start_ticks));
      }
    }
  }

  SDL_Quit();

  return 0;
}
