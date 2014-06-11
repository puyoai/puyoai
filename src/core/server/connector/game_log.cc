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
