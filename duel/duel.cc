#include <cstdlib>
#include <iostream>
#include <fstream>
#include <glog/logging.h>
#include <map>
#include <sstream>
#include <string>

#include "data.h"
#include "decision.h"
#include "game.h"
#include "game_log.h"

#ifdef __MINGW32__
#include <windows.h>
#include "connector_manager_windows.h"
#else
#include "connector_manager_linux.h"
#endif

#ifdef USE_SDL
// We need to replace main on Mac.
# include <SDL.h>
#endif

using namespace std;

void SendInfo(ConnectorManagerBase* manager, int id, string status[2]) {
  for (int i = 0; i < 2; i++) {
    stringstream ss;
    ss << "ID=" << id << " "
       << status[i].c_str();
    manager->Write(i, ss.str());
  }
}

GameLog duel(ConnectorManagerBase* manager, int* scores) {
  LOG(INFO) << "Cpu managers started.";
  Game game;
  LOG(INFO) << "Game started.";
  int current_id = 0;

  GameLog game_log;
  while (true) {
    // Timeout is 120s, and the game is 30fps.
    if (current_id >= FPS * 120) {
      game_log.result = DRAW;
      return game_log;
    }
    // GO TO THE NEXT FRAME.
    current_id++;
    string player_info[2];
    game.GetFieldInfo(&player_info[0], &player_info[1]);
    SendInfo(manager, current_id, player_info);

    // CHECK IF THE GAME IS OVER.
    GameResult result = game.GetWinner(scores);
    if (result != PLAYING) {
      game_log.result = result;
      return game_log;
    }

    // READ INFO.
    // It takes up to 16ms to finish this section.
    vector<PlayerLog> all_data;
    if (!manager->GetActions(current_id, &all_data)) {
      if (manager->IsConnectorAlive(0)) {
        game_log.result = P1_WIN_WITH_CONNECTION_ERROR;
        game_log.error_log = manager->GetErrorLog();
        return game_log;
      } else {
        game_log.result = P2_WIN_WITH_CONNECTION_ERROR;
        game_log.error_log = manager->GetErrorLog();
        return game_log;
      }
    }

    // PLAY.
    game.Play(all_data, &game_log);
  }
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  if (argc != 3) {
    LOG(ERROR) << "There must be 2 arguments." << endl;
    return 1;
  }

  // Initialize Randomization.
  const char* seed_str = getenv("PUYO_SEED");
  unsigned int seed = seed_str ? atoi(seed_str) : time(NULL);
  srand(seed);
  std::cout << "seed=" << seed << std::endl;
  LOG(INFO) << "seed=" << seed;

  // Clear the screen.
  const string CLEAR = "\x1b[2J";
  std::cout << CLEAR;

  // Start the game.
  vector<string> program_names;
  program_names.push_back(string(argv[1]));
  program_names.push_back(string(argv[2]));

  const char* num_duel_str = getenv("PUYO_NUM_DUEL");
  int num_duel = num_duel_str ? atoi(num_duel_str) : -1;
  const char* num_win_str = getenv("PUYO_NUM_WIN");
  int num_win = num_win_str ? atoi(num_win_str) : -1;

  LOG(INFO) << "Starting duel server.";
#ifdef __MINGW32__
  ConnectorManagerWindows manager(program_names);
#else
  ConnectorManagerLinux manager(program_names);
#endif

  int p1_win = 0;
  int p1_draw = 0;
  int p1_lose = 0;

  bool log_result = false;
  string log_dir_str;
  const char* log_dir = getenv("PUYO_LOG_DIRECTORY");
  if (log_dir) {
    log_result = true;
    log_dir_str = string(log_dir);
    if (log_dir_str.substr(log_dir_str.size() - 1, 1) != "/") {
      log_dir_str += "/";
    }
  }

  int num_match = 0;
  while (true) {
    int scores[2];
    GameLog log = duel(&manager, scores);

    if (log_result) {
      stringstream filename;
      filename << log_dir_str << num_match;
      ofstream ofs(filename.str().c_str(), ios::out);
      string output;
      log.SerializeToString(&output);
      ofs << "var data = " << output;
      ofs.close();
    }

    string filename;
    if (log_result) {
      stringstream ss;
      ss << log_dir_str << num_match;
      filename = ss.str();
    }

    string result = "";
    switch (log.result) {
      case P1_WIN:
        p1_win++;
        result = "P1_WIN";
        break;
      case P2_WIN:
        p1_lose++;
        result = "P2_WIN";
        break;
      case DRAW:
        p1_draw++;
        result = "DRAW";
        break;
      case P1_WIN_WITH_CONNECTION_ERROR:
        result = "P1_WIN_WITH_CONNECTION_ERROR";
        break;
      case P2_WIN_WITH_CONNECTION_ERROR:
        result = "P2_WIN_WITH_CONNECTION_ERROR";
        break;
    }

    map<string, string> results;
    results["result"] = result;
    results["logfile"] = filename;
    const char* user1 = getenv("PUYO_USER1");
    const char* user2 = getenv("PUYO_USER2");
    if (user1) results["user1"] = string(user1);
    if (user2) results["user2"] = string(user2);

    int is_first = true;
    cerr << "{";
    for (map<string, string>::iterator i = results.begin(); i != results.end(); i++) {
      if (!is_first) {
        cerr << ", ";
      }
      is_first = false;
      cerr << "\"" << i->first << "\": \"" << i->second << "\"";
    }
    cerr << "}" << endl;

    LOG(INFO) << result;
    cout << result << endl;

    cout << p1_win << " / " << p1_draw << " / " << p1_lose
         << " " << scores[0] << " vs " << scores[1] << endl;

    seed = rand();
    srand(seed);
    LOG(INFO) << "seed=" << seed;

    if (log.result == P1_WIN_WITH_CONNECTION_ERROR ||
        log.result == P2_WIN_WITH_CONNECTION_ERROR ||
        p1_win == num_win || p1_lose == num_win ||
        p1_win + p1_draw + p1_lose == num_duel) {
      break;
    }
    num_match++;
  }
}
