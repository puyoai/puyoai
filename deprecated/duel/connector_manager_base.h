#ifndef DUEL_CONNECTOR_MANAGER_BASE_H_
#define DUEL_CONNECTOR_MANAGER_BASE_H_

#include <string>
#include <vector>

#include "game_log.h"

struct Data;

class ConnectorManagerBase {
 public:
  ConnectorManagerBase() {};
  virtual void Write(int id, std::string message) = 0;
  virtual bool GetActions(
      int frame_id, std::vector<PlayerLog>* all_data) = 0;
  virtual bool IsConnectorAlive(int id) = 0;
  virtual std::string GetErrorLog() = 0;
};

#endif  // DUEL_CONNECTOR_MANAGER_BASE_H_
