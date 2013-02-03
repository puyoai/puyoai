#ifndef __CONNECTOR_MANAGER_LINUX_H__
#define __CONNECTOR_MANAGER_LINUX_H__

#include <poll.h>
#include <vector>

#include "connector.h"
#include "connector_manager_base.h"
#include "data.h"

const int TIMEOUT_USEC = 1000000 / FPS;

class ConnectorManagerLinux : public ConnectorManagerBase {
 public:
  ConnectorManagerLinux(std::vector<std::string> program_names);
  virtual void Write(int id, std::string message);
  virtual bool GetActions(
      int frame_id, std::vector<PlayerLog>* all_data);
  virtual bool IsConnectorAlive(int id);
  virtual std::string GetErrorLog();
  void DontWaitTimeout();

 private:
  Connector CreateConnector(std::string program_name, int id);

  std::vector<Connector> connectors_;
  pollfd pollfds_[2];
  std::vector<bool> connector_is_alive_;
  bool dont_wait_timeout_;
};

#endif  // __CONNECTOR_MANAGER_LINUX_H__
