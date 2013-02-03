#ifndef __DATA_H__
#define __DATA_H__

#include <string>

typedef enum ConnectionStatus {
  UNREAD,
  OK,
  DIE,
} ConnectionStatus;

struct Data {
  int id;
  int x;
  int r;
  int usec;
  std::string original;
  std::string msg;
  std::string mawashi_area;
  ConnectionStatus status;
  static Data NO_INPUT;
  static Data USE_LAST_INPUT;

  Data() {
    status = UNREAD;
    id = 0;
    x = 0;
    r = 0;
  }

  bool HasDecision() const {
    if (x != 0 || r != 0) {
      return true;
    }
    return false;
  }

  bool IsValid() const {
    if (status == OK) {
      if (x == 0 && r == 0) {
        return true;
      }
      if (x == 1 && r == 3) {
        return false;
      }
      if (x == 6 && r == 1) {
        return false;
      }
      return (1 <= x && x <= 6 && 0 <= r && r <= 3);
    }
    return false;
  }
};

#endif  // __DATA_H__
