#include "game_log.h"

#include <sstream>
#include <string>

using namespace std;

void ReceivedData::SerializeToString(string* output) const {
  stringstream ss;
  ss << "{";
  if (!original.empty()) {
    ss << "'original':";
    ss << "'" << original << "'";
    ss << ",";
  }
  if (decision.x != 0 || decision.r != 0) {
    ss << "'decision':";
    ss << "{'x':" << decision.x << ",'r':" << decision.r << "}";
    ss << ",";
  }
  ss << "'timestamp':";
  ss << timestamp;
  ss << ",";
  ss << "'frame_id':";
  ss << frame_id;
  ss << "}";
  output->append(ss.str());
}

void ExecutionData::SerializeToString(string* output) const {
  stringstream ss;
  ss << "{";
  ss << "'keys':";
  ss << "[";
  for (size_t i = 0; i < keys.size(); i++) {
    if (i > 0) {
      ss << ",";
    }
    ss << keys[i];
  }
  ss << "],";
  ss << "moving:{";
  ss << "'x':" << moving.x << ",";
  ss << "'y':" << moving.y << ",";
  ss << "'r':" << moving.r << ",";
  ss << "'c1':" << moving.color[0] << ",";
  ss << "'c2':" << moving.color[1];
  ss << "}";

  bool has_ojama = false;
  for (int i = 0; i < 6; i++) {
    if (ojama[i] != 0) {
      has_ojama = true;
    }
  }
  if (has_ojama) {
    ss << ",";
    ss << "'ojama':[";
    for (int i = 0; i < 6; i++) {
      if (i != 0) {
        ss << ",";
      }
      ss << ojama[i];
    }
    ss << "]";
  }
  if (landed) {
    ss << ",";
    ss << "'landed':1";
  }
  ss << "}";
  output->append(ss.str());
}

void PlayerLog::SerializeToString(string* output) const {
  stringstream ss;
  ss << "\n";
  ss << "{";
  ss << "'frame_id':";
  ss << frame_id;
  ss << ",";
  ss << "'player_id':";
  ss << player_id;
  ss << ",";
  ss << "'received_data':[";
  for (size_t i = 0; i < received_data.size(); i++) {
    if (i > 0) {
      ss << ",";
    }
    string tmp;
    received_data[i].SerializeToString(&tmp);
    ss << tmp;
  }
  ss << "]";
  ss << ",";
  ss << "'execution_data':";
  {
    string tmp;
    execution_data.SerializeToString(&tmp);
    ss << tmp;
  }
  ss << "}";
  output->append(ss.str());
}

void GameLog::SerializeToString(string* output) const {
  stringstream ss;
  ss << "{";
  ss << "'GameResult':" << result << ",";
  ss << "'error_log':'" << error_log << "',";
  ss << "'log':[";
  for (size_t i = 0; i < log.size(); i++) {
    if (i > 0) {
      ss << ",";
    }
    string tmp;
    log[i].SerializeToString(&tmp);
    ss << tmp;
  }
  ss << "]";
  ss << "}";
  output->append(ss.str());
}
