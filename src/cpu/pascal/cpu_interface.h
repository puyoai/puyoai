#include <string>

#include "deprecated_field.h"

class Decision;

class Cpu {
public:
  class PlayerData {
   public:
    PlayerData() : state(0), yokoku("000000") {
    }

    // Position of falling puyo.
    int x;
    int y;
    int r;
    // Field information.
    FieldWithColorSequence field;
    // block of flags. Use some functions defined below. The flags are defined
    // in core/state.h.
    int state;
    // Colors of current puyos, next puyos double next puyos in human readable
    // style (colors are '0', '4' - '7', and defined in core/field.h).
    // '4' - '7' represents colored puyo. '0' represents the color is unknown
    // (it happens before double next puyo appears).
    std::string yokoku;
    // How many Ojama puyos are displayed on the top. It is a sum of fixed
    // ojamas (chain has finished) and pending ojamas (chain is ongoing).
    int ojama;
  };

  class Data {
   public:
    // Frame ID.
    int id;
    PlayerData player[2];

    // Raw data of sent message. Usually you are not interested in the data,
    // because the data is parsed into class Data.
    std::string original;

    Data() : id(0) {
    };

    enum {
      FIELD_STATE_NONE = 0,
      FIELD_STATE_YOU_CAN_PLAY = 1 << 0,
      FIELD_STATE_WNEXT_APPEARED = 1 << 2,
      FIELD_STATE_YOU_GROUNDED = 1 << 4,
      FIELD_STATE_YOU_WIN = 1 << 6,
      FIELD_STATE_CHAIN_DONE = 1 << 8,
    };

    bool HasControl(int user_id) const {
      return player[user_id].state & FIELD_STATE_YOU_CAN_PLAY;
    }
    bool Ground(int user_id) const {
      return player[user_id].state & FIELD_STATE_YOU_GROUNDED;
    }
    bool Win(int user_id) const {
      return player[user_id].state & FIELD_STATE_YOU_WIN;
    }
    bool ChainDone(int user_id) const {
      return player[user_id].state & FIELD_STATE_CHAIN_DONE;
    }
  };

  void Run();

 private:
  // You should implement this function.
  // When you want to move your puyo, decision must be filled.
  // message is an optional parameter. When you fill the string, the message
  // will be displayed on matching server.
  // Note: do not update the message too often. The field will be updated 30
  // times per second.
  virtual void GetDecision(
      const Data& data, Decision* decision, std::string* message) = 0;

  void ReceiveCurrentStatus(Data* data) const;
  void SendDecision(
      const Data& data, const Decision& decision,
      const std::string& message)const;
};
