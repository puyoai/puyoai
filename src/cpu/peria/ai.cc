#include "cpu/peria/ai.h"

#include "core/constant.h"
#include "core/field/core_field.h"
#include "core/field/rensa_result.h"

namespace {

bool IsQuick(RensaResult result) {
  int frame = result.frames;
  frame -= FRAMES_AFTER_NO_DROP;
  frame -= FRAMES_AFTER_VANISH * result.chains;
  frame -= FRAMES_AFTER_DROP * (result.chains - 1);
  return frame % FRAMES_DROP_1_LINE == 0;
}

}  // namespace

namespace peria {

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int /*frame_id*/,
		       const PlainField& /*field*/,
		       const Kumipuyo& /*next1*/,
		       const Kumipuyo& /*next2*/) {
  return DropDecision();
}

void Ai::gameWillBegin(const FrameData& /*frame_data*/) {
  zenkeshi_[0] = false;
  zenkeshi_[1] = false;
}

void Ai::enemyGrounded(const FrameData& frame_data) {
  const PlayerFrameData& enemy = frame_data.enemyPlayerFrameData();

  CoreField field(enemy.field);
  RensaResult result = field.simulate();
  if (result.score > 0) {
    bool is_quick = IsQuick(result);
    // attack_.end_at = frame_data.frame_id + result.frames;
    // attack_.tsubushi = Is潰し();
    // attack_.saisoku = Is催促(); // 連鎖数と続き部分からの連鎖数で判断
    // attack_.honsen = Is本線();  // 消えたぷよ数で判断
  } else {
    // attack_.Reset();
    // 3 手完全探索する
    // 発火可能な最大点数とその発火までの frame 数を記録
  }
}

}  // namespace peria
