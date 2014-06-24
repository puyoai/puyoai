#include "cpu/peria/ai.h"

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

void Ai::enemyGrounded(const FrameData& /*frame_data*/) {
  // TODO:
  // if (消えた場合) {
  //   attack_.frame_to_end = 計算;
  //   attack_.score = 計算;
  //   attack_.tsubushi = Is潰し();
  //   attack_.saisoku = Is催促(); // 連鎖数と続き部分からの連鎖数で判断
  //   attack_.honsen = Is本線();  // 消えたぷよ数で判断
  // } else {
  //   attack_.Reset();
  //   3 手完全探索する
  //   発火可能な最大点数とその発火までの frame 数を記録
  // }
}

}  // namespace peria
