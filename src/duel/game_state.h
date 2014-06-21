#ifndef DUEL_GAME_STATE_H_
#define DUEL_GAME_STATE_H_

#include <string>
#include <glog/logging.h>

#include "duel/field_realtime.h"

class GameState {
public:
    GameState(const FieldRealtime& p1Field, const FieldRealtime& p2Field,
              const std::string& p1Message, const std::string& p2Message) :
        field_ { p1Field, p2Field },
        message_ { p1Message, p2Message }
    {
    }

    const FieldRealtime& field(int i) const { return field_[i]; }
    const std::string& message(int i) const { return message_[i]; }

    std::string toJson() const;

private:
    FieldRealtime field_[2];
    std::string message_[2];
};

#endif
