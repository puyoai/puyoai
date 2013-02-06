#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <string>

#include "base.h"
#include "game.h"

namespace {
void Echo() {
  string str;
  getline(cin, str, '\n');
  cout << str << endl;
}
}  // namespace

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Make sure the CPU is connected to the duel server.
  // Echo back what we receive.
  Echo();

  while (!cin.eof()) {
    string input;
    scoped_ptr<Game> game(new Game(argv[1]));
    while (getline(cin, input) && game->Input(input))
      cout << game->Play() << endl;
  }

  return 0;
}
