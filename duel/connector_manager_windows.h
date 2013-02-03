#ifndef __CONNECTOR_MANAGER_WINDOWS_H__
#define __CONNECTOR_MANAGER_WINDOWS_H__

#include <string>
#include <vector>

#include "connector_manager_base.h"
#include "game_log.h"

class Data;
class Hikuitokoro;

class ConnectorManagerWindows : public ConnectorManagerBase {
 public:
  ConnectorManagerWindows(std::vector<std::string> program_names);
  ~ConnectorManagerWindows();
  virtual void Write(int id, std::string message);
  virtual bool GetActions(
      int frame_id, std::vector<PlayerLog>* all_data);
 private:
  Hikuitokoro* ai_[2];
  std::string lines_[2];
};

#endif  // __CONNECTOR_MANAGER_WINDOWS_H__
