#ifndef DUEL_FIELD_REALTIME_H_
#define DUEL_FIELD_REALTIME_H_

#include "field.h"

#include <vector>

#include "duel/game_log.h"

const int ZENKESI_BONUS = 2100;

class OjamaController;

class FieldRealtime : public Field {
 public:
  FieldRealtime(int player_id, const std::string& color_sequence,
                OjamaController* ojama_ctrl);
  void Init();
  // Gives a key input to the field, and control puyo. Returns true if a key
  // input is accepted.
  bool Play(Key key, PlayerLog* player_log);
  // Pretty print of the field.
  void Print() const;
  void Print(const std::string& debug_message) const;
  // Checks if a player is dead.
  bool IsDead() const;
  void GetCurrentPuyo(int* x1, int* y1, char* c1,
                      int* x2, int* y2, char* c2, int* r) const;

  // Utility functions to be used by duel server.
  std::string GetFieldInfo() const;
  std::string GetYokokuInfo() const;
  int GetStateInfo() const;
  int GetScore() const;
  Key GetKey(const Decision& decision);

  int player_id() const { return player_id_; }
  bool IsInUserState() const { return simulate_real_state_ == STATE_USER; }
  int GetFixedOjama() const;
  int GetPendingOjama() const;

  enum {
    STATE_USER,
    STATE_CHIGIRI,
    STATE_VANISH,
    STATE_DROP,
    STATE_OJAMA,
    STATE_SLEEP,
  };

  // Testing only.
  int GetSimulationState() const;

 private:
  bool Chigiri();
  bool Drop1line();
  bool PlayInternal(Key key, bool* ground);
  void PrepareNextPuyo();
  void FinishChain();
  bool TryChigiri();
  bool TryVanish();
  bool TryDrop();
  bool TryOjama(PlayerLog* player_log);

  OjamaController* ojama_ctrl_;
  bool ojama_dropping_;
  std::vector<int> ojama_position_;
  int player_id_;
  int x_;
  int y_;
  int r_;
  int sleep_for_;
  bool drop_animation_;
  int simulate_real_state_;
  int chigiri_x_;
  int chigiri_y_;
  bool is_dead_;
  int field_state_;
  int frames_for_free_fall_;
  int score_;
  int consumed_score_;
  int current_chains_;
  int quickturn_;
  bool is_zenkesi_;
  int dropped_rows_;

  bool delay_double_next_;
  int yokoku_delay_;
};

#endif  // DUEL_FIELD_REALTIME_H_
