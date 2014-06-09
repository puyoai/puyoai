#include "sample.h"

#include <fstream>
#include <string>

#include "core/decision.h"
#include "core/ctrl.h"

using namespace std;

void SampleCpu::GetDecision(
    const Data& data, Decision* decision, string* message) {
  // This message will be displayed on matching simulator.
  *message = "This is a test message";

  // x-coordinate of Jiku puyo.
  decision->x = 1;
  // Rotation.
  //   0: Ko-puyo is above Jiku-puyo.
  //   1: Ko-puyo is on the right side of Jiku-puyo.
  //   2: Ko-puyo is below Jiku-puyo.
  //   3: Ko-puyo is on the left side of Jiku-puyo.
  decision->r = 1;
  if (Ctrl::isReachable(data.player[0].field, *decision)) {
    return;
  }
  decision->x = 6;
  decision->r = 3;
  if (Ctrl::isReachable(data.player[0].field, *decision)) {
    return;
  }
  decision->x = 3;
  decision->r = 1;
  return;
}

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  // Logging.
  string name = "/tmp/" + string(argv[1]) + ".txt";
  ofstream ofs(name.c_str());

  SampleCpu cpu;
  cpu.Run();
}
