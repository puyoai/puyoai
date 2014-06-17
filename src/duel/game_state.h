#ifndef DUEL_GAME_STATE_H_
#define DUEL_GAME_STATE_H_

#include <string>
#include <glog/logging.h>

#include "duel/field_realtime.h"

class GameState {
public:
    GameState(const FieldRealtime& p1Field, const FieldRealtime& p2Field,
              const std::string& p1Message, const std::string& p2Message) :
        p1Field_(p1Field), p2Field_(p2Field),
        p1Message_(p1Message), p2Message_(p2Message)
    {
    }

    const FieldRealtime& field(int i) const {
        switch (i) {
        case 0:
            return p1Field_;
        case 1:
            return p2Field_;
        default:
            CHECK(false);
            return p1Field_;
        }
    }

    const std::string& message(int i) const {
        switch (i) {
        case 0:
            return p1Message_;
        case 1:
            return p2Message_;
        default:
            CHECK(false);
            return p1Message_;
        }
    }

    std::string toJson() const;

private:
    const FieldRealtime p1Field_;
    const FieldRealtime p2Field_;
    const std::string p1Message_;
    const std::string p2Message_;
};

#endif
