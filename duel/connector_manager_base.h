#ifndef __CONNECTOR_MANAGER_BASE_H__
#define __CONNECTOR_MANAGER_BASE_H__

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

#endif  // __CONNECTOR_MANAGER_BASE_H__
