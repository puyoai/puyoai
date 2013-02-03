#include "connector_manager_windows.h"

#include <sstream>
#include <string>
#include <vector>

#include "core/data.h"
#include "../cpu/hiroshimizuno/hikuitokoro.h"

using std::istringstream;
using std::string;
using std::vector;

ConnectorManagerWindows::ConnectorManagerWindows(std::vector<std::string> program_names) {
  ai_[0] = new Hikuitokoro();
  ai_[1] = new Hikuitokoro();
}
ConnectorManagerWindows::~ConnectorManagerWindows() {
  delete ai_[0];
  delete ai_[1];
}

void ConnectorManagerWindows::Write(int id, std::string message) {
  lines_[id] = message;
}

bool ConnectorManagerWindows::GetActions(
    int frame_id, vector<PlayerLog>* all_data) {
  all_data->clear();
  for (int player = 0; player < 2; ++player) {
    string response = ai_[player]->ProcessFrame(lines_[player]);
    istringstream is(response);
    int frame_id;
    int x, r;
    d.original = response;
    char c;
    is >> c >> c >> c >> frame_id >> c >> c >> x >> c >> c >> r;
    ReceivedData received_data;
    received_data.original = response;
    received_data.frame_id = frame_id;
    received_data.decision = Decision(x, r);
    (*all_data)[player].received_data.push_back(received_data);
  }
  return true;
}
