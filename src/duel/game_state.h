#ifndef DUEL_GAME_STATE_H_
#define DUEL_GAME_STATE_H_

#include <string>
#include <glog/logging.h>

#include "base/base.h"
#include "core/frame_request.h"
#include "core/game_result.h"
#include "duel/field_realtime.h"

class GameState {
public:
    explicit GameState(const KumipuyoSeq& seq);

    GameResult gameResult() const;
    std::string toJson() const;
    std::string toDebugString() const;

    int frameId() const { return frameId_; }
    void setFrameId(int frameId) { frameId_ = frameId; }

    FrameRequest toFrameRequest(int playerId, int frameId) const;
    FrameRequest toFrameRequest(int playerId, int frameId, GameResult forceSetGameResult) const;

    const FieldRealtime& field(int pi) const { return field_[pi]; }
    FieldRealtime* mutableField(int pi) { return &field_[pi]; }

    const Decision& decision(int pi) const { return decision_[pi]; }
    void setDecision(int pi, const Decision& decision) { decision_[pi] = decision; }
    Decision* mutableDecision(int pi) { return &decision_[pi]; }

    const std::string& message(int pi) const { return message_[pi]; }
    void setMessage(int pi, const std::string& message) { message_[pi] = message; }

    void setAckFrameId(int pi, int frameId) { ackFrameId_[pi] = frameId; }
    int ackFrameId(int pi) const { return ackFrameId_[pi]; }

    void setNackFrameIds(int pi, const std::vector<int>& frameIds) { nackFrameIds_[pi] = frameIds; }
    const std::vector<int>& nackFrameIds(int pi) const { return nackFrameIds_[pi]; }

private:
    int frameId_ = 0;
    FieldRealtime field_[2];
    Decision decision_[2];
    std::string message_[2];
    int ackFrameId_[2];
    std::vector<int> nackFrameIds_[2];
};

#endif
