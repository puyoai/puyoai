#ifndef CPU_PERIA_H_
#define CPU_PERIA_H_

#include "core/client/ai/ai.h"

namespace peria {

class Ai : public ::AI {
 public:
  Ai();
  virtual ~Ai();

 protected:
  virtual DropDecision think(int frame_id,
                             const PlainField& field,
                             const KumipuyoSeq& seq) override;
  virtual void gameWillBegin(const FrameData& frame_data) override;
  virtual void enemyGrounded(const FrameData& frame_data) override;

 private:
  bool zenkeshi_[2];
};

}  // namespace peria

#endif // CPU_PERIA_H_
