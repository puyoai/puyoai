#include <fstream>  // for logging.
using std::ofstream;

#include <iostream>
#include <string>

#include "frame.h"
#include "linux.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

bool ReadLine(string* line) {
  if (CanReadStdin() > 0) {
    getline(cin, *line);
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {
  ofstream log; log.open("/tmp/maton.txt");
  log << MicroSeconds() << endl;
  //unsigned long long time_waiting = 0;
  //unsigned long long time_reading = 0;
  string line;
  Frame frame = {0};
  int decision_x = 1;
  int decision_r = 0;
  //unsigned long long time_last = MicroSeconds();
  while (true) {
    if (ReadLine(&line)) {
      if (ParseLine(line, &frame)) {
        log << "[L] " << line << endl;
        if (frame.id == 1) {
          // Adjust parameters according to the win/loss rate.
          decision_x = 4442 - frame.lose;
          decision_x %= 5;
          decision_x += 2;
          decision_r = 2;
        } else if (frame.ack < frame.nack) {
          // Send plan-B action here.
          decision_x = 1;
          decision_r = 0;
          log << "[SECOND] " << "ack=" << frame.ack << ",nack=" << frame.nack << endl;
          log << "[ACTION] " << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
          cout << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
        } else if (frame.state_canmove) {
          // Calculate here.
          //decision_x = 5;
          //decision_r = 1;
          log << "[ACTION] " << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
          cout << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
        } else {
          // We need to tell the decision even though we have already told it to core/duel.
          log << "[ACTION] " << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
          cout << "ID=" << frame.id << " X=" << decision_x << " R=" << decision_r << endl;
        }
      } else {
        if (line == "PingMessage") { cout << "PongMessage" << endl; }
        else { log << "[BADLINE] " << line << endl; }
      }
    }
    // Pre-calculate here.
  }
  // Should never reach here.
  log.close();
  return -1;
}
