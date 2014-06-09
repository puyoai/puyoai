#ifndef __DATA_H__
#define __DATA_H__

#include <fstream>
#include <sstream>
#include <string>

#include "field.h"

enum {
  FIELD_STATE_NONE = 0,
  FIELD_STATE_YOU_CAN_PLAY = 1 << 0,
  FIELD_STATE_WNEXT_APPEARED = 1 << 2,
  FIELD_STATE_YOU_GROUNDED = 1 << 4,
  FIELD_STATE_YOU_WIN = 1 << 6,
  FIELD_STATE_CHAIN_DONE = 1 << 8,
};

class UserData {
 public:
  int x;
  int y;
  int r;
  Field field;
  int state;
  std::string yokoku;
  int ojama;
  int score;

  bool HasControl() {
    return state & FIELD_STATE_YOU_CAN_PLAY;
  }
  bool Ground() {
    return state & FIELD_STATE_YOU_GROUNDED;
  }
  bool Win() {
    return state & FIELD_STATE_YOU_WIN;
  }
  bool HasChainDone() {
    return state & FIELD_STATE_CHAIN_DONE;
  }
};

class Data {
 public:
  int id;
  int ack;
  std::vector<int> nack;
  UserData users[2];
  std::string original;

  Data() {
    id = 0;
  };

  static Data Get(std::ofstream& ofs) {
    Data data;
    std::string str;
    getline(std::cin, str, '\n');

    std::istringstream iss(str);
    std::string tmp;
    data.original = std::string(str);
    ofs << str << std::endl << std::flush;
    while(getline(iss, tmp, ' ')) {
      if (tmp.substr(0, 3) == "ID=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.id;
      } else if (tmp.substr(0, 3) == "YX=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[0].x;
      } else if (tmp.substr(0, 3) == "OX=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[1].x;
      } else if (tmp.substr(0, 3) == "YY=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[0].y;
      } else if (tmp.substr(0, 3) == "OY=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[1].y;
      } else if (tmp.substr(0, 3) == "YR=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[0].r;
      } else if (tmp.substr(0, 3) == "OR=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[1].r;
      } else if (tmp.substr(0, 3) == "YP=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[0].yokoku;
      } else if (tmp.substr(0, 3) == "OP=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[1].yokoku;
      } else if (tmp.substr(0, 3) == "YF=") {
        std::istringstream istr(tmp.c_str() + 3);
        std::string yf;
        istr >> yf;
        data.users[0].field = Field(yf);
      } else if (tmp.substr(0, 3) == "OF=") {
        std::istringstream istr(tmp.c_str() + 3);
        std::string of;
        istr >> of;
        data.users[1].field = Field(of);
      } else if (tmp.substr(0, 3) == "YO=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[0].ojama;
      } else if (tmp.substr(0, 3) == "OO=") {
        std::istringstream istr(tmp.c_str() + 3);
        istr >> data.users[1].ojama;
      } else if (tmp.substr(0, 4) == "ACK=") {
        std::istringstream istr(tmp.c_str() + 4);
        istr >> data.ack;
      } else if (tmp.substr(0, 4) == "NACK=") {
        std::istringstream istr(tmp.c_str() + 4);
        std::string tmp;
        while(getline(istr, tmp, ',')) {
          data.nack.push_back(atoi(tmp.c_str()));
        }
      } else if (tmp.substr(0, 6) == "STATE=") {
        std::istringstream istr(tmp.c_str() + 6);
        int state;
        istr >> state;
        data.users[0].state = (state >> 0) & 0x55555555;
        data.users[1].state = (state >> 1) & 0x55555555;
      }
    }
    return data;
  }
};

#endif  // __DATA_H__
