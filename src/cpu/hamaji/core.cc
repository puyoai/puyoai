#include "core.h"

#include <stdio.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/score.h"

#include "eval.h"
#include "eval2.h"
#include "game.h"
#include "util.h"

DEFINE_int32(immediate_fire_score, 40000, "");
DEFINE_bool(use_next_next, true, "");
DEFINE_bool(eval2, false, "");

Core::Core(bool is_solo)
  : is_solo_(is_solo),
    best_chain_(0),
    best_score_(0) {
  eval_ = new Eval();
  eval2_ = new Eval2();
}

Core::~Core() {
  delete eval_;
  delete eval2_;
}

int numTurns(const LP& r) {
  const LP* p = r.parent;
  int n = 0;
  while (p) {
    p = p->parent;
    n++;
  }
  return n;
}

Decision Core::decide(Game* game) {
  LOG(INFO) << game->getDebugOutput();

  if (is_solo_) {
    msg_.clear();
  }

  vector<LP> plans;
  game->p[0].f.FindAvailablePlans(game->p[0].next, &plans);

  if (game->p[1].expected_ojama > 3) {
    int best_score = 400;
    const LP* best_plan = NULL;
    // TODO(hamaji): Adjust timing.
    for (size_t i = 0; i < plans.size(); i++) {
      int t = numTurns(plans[i]);
      if (t != 0 && (t - 1) * 70 + 20 > game->p[1].expected_frame)
        continue;
      if (best_score < plans[i].score) {
        best_score = plans[i].score;
        best_plan = &plans[i];
      }
    }
    int my_ojama = best_plan ? best_score / 70 : 0;
    if (best_plan) {
      if ((game->p[1].expected_frame < 100 &&
           my_ojama + 50 > game->p[1].expected_ojama) ||
          (game->p[1].expected_frame < 500 &&
           my_ojama > game->p[1].expected_ojama) ||
          my_ojama - 120 > game->p[1].expected_ojama) {
        msg_ = ssprintf("COUNTER_%d_vs_%d_in_%d",
                        my_ojama, game->p[1].expected_ojama,
                        game->p[1].expected_frame);
        LOG(INFO) << "Found a neat counter! score=" << best_score
                  << " ojama=" << game->p[1].expected_ojama
                  << " frames=" << game->p[1].expected_frame
                  << " my_ojama=" << my_ojama
                  << '\n' << best_plan->field.GetDebugOutput();
        return best_plan->getFirstDecision();
      }
    }
    LOG(INFO) << "No counter..."
              << " ojama=" << game->p[1].expected_ojama
              << " frames=" << game->p[1].expected_frame
              << " my_ojama=" << my_ojama;
  }

  if (!is_solo_) {
    int best_score = 0;
    const LP* best_plan = NULL;
    for (size_t i = 0; i < plans.size(); i++) {
      int score = plans[i].score;
      if (plans[i].chain_cnt < 4 && !plans[i].field.hasPuyo())
        score += 2100;
      if (best_score < score) {
        best_score = score;
        best_plan = &plans[i];
      }
    }
    if (best_plan &&
        best_score / 70 - 30 > game->p[1].expected_ojama &&
        (game->p[0].f.Get(3, 11) != PuyoColor::EMPTY ||
         (game->p[0].f.Get(2, 12) != PuyoColor::EMPTY && game->p[0].f.Get(4, 11) != PuyoColor::EMPTY) ||
         (game->p[0].f.Get(2, 11) != PuyoColor::EMPTY && game->p[0].f.Get(4, 12) != PuyoColor::EMPTY) ||
         best_score > FLAGS_immediate_fire_score)) {
      msg_ = ssprintf("IMMEDIATE_%d", best_score);
      LOG(INFO) << "Immediate fire! score=" << best_score
                << '\n' << best_plan->field.GetDebugOutput();
      return best_plan->getFirstDecision();
    }

    if (best_plan && best_score > 1000) {
      int puyo_cnt = game->p[1].f.countColorPuyo();
      int max_chain_cnt =
          min((puyo_cnt + best_plan->chain_cnt * 2 + 3) / 4, 19);
      int max_score = 0;
      for (int i = 2; i <= max_chain_cnt; i++) {
        max_score += chainBonus(i);
      }
      max_score *= 40;
      max_score += game->p[1].score - game->p[1].spent_score;
      LOG(INFO) << "Kill move? best_score=" << best_score
                << " chain_cnt=" << best_plan->chain_cnt
                << " opp_max_score=" << max_score
                << " opp_chain_cnt=" << max_chain_cnt
                << " opp_puyo_cnt=" << puyo_cnt;
      if (best_score > max_score + game->p[1].expected_ojama * 70) {
        msg_ = ssprintf("KILL_%d", best_score);
        LOG(INFO) << "Kill move! score=" << best_score
                  << " vs " << max_score
                  << '\n' << best_plan->field.GetDebugOutput();
        return best_plan->getFirstDecision();
      }
    }

    // TODO(hamaji): Maybe better not to take zenkeshi when the
    //               opponent has a lot of puyos or firing big chain.
#if 0
    if (best_plan)
      LOG(INFO) << "ZENKESHI??? best_score=" << best_score
                << " chain_cnt=" << best_plan->chain_cnt;
#endif
    if (best_plan && best_plan->chain_cnt < 4 &&
        !best_plan->field.hasPuyo()) {
      LOG(INFO) << "ZENKESHI! best_score=" << best_score
                << " chain_cnt=" << best_plan->chain_cnt;
      return best_plan->getFirstDecision();
    }
  } else {
    for (size_t i = 0; i < plans.size(); i++) {
      for (const LP* p = &plans[i]; p; p = p->parent) {
        int score = p->score - (p->parent ? p->parent->score : 0);
        if (best_score_ < score)
          best_score_ = score;
        int chain_cnt = p->chain_cnt - (p->parent ? p->parent->chain_cnt : 0);
        if (best_chain_ < chain_cnt) {
          best_chain_ = chain_cnt;
          if (is_solo_) {
            printf("Got the best chain: %d score=%d\n",
                   best_chain_, best_score_);
          } else {
            LOG(INFO) << "Got the best chain: " << best_chain_
                      << " score=" << best_score_;
          }
        }
      }
    }
  }

  if (game->p[1].event.grounded) {
    vector<LP> oplans;
    game->p[1].f.FindAvailablePlans(game->p[1].next.subsequence(2), 2, &oplans);
    int obest_score = 0;
    for (size_t i = 0; i < oplans.size(); i++) {
      if (obest_score < oplans[i].score)
        obest_score = oplans[i].score;
    }
    obest_score += game->p[1].expected_ojama * 70;

    const LP* best_plan = NULL;
    int best_score = 0;
    for (size_t i = 0; i < plans.size(); i++) {
      const LP* p = &plans[i];
      if (p->parent) {
        continue;
      }

      if (p->chain_cnt <= 2 && p->score - obest_score > 1200) {
        best_score = p->score;
        best_plan = p;
      }
    }

    if (best_plan) {
      msg_ = ssprintf("TSUBUSHI_%d", best_score);
      LOG(INFO) << "Tsubushi! score=" << best_score
                << '\n' << best_plan->field.GetDebugOutput();
      return best_plan->getFirstDecision();
    }
  }

  int ojama = game->p[1].expected_ojama;
  int ojama_height = 0;
  if (ojama > 3) {
    ojama_height = (ojama + 5) / 6;
    if (ojama_height >= 12 || game->p[0].f.Get(3, 12-ojama_height) != PuyoColor::EMPTY)
      ojama_height = 0;
  }

 retry:
  double best_value = -1e9;
  const LP* best_plan = NULL;
  for (size_t i = 0; i < plans.size(); i++) {
    LP* p = &plans[i];
    if (!FLAGS_use_next_next && p->parent && p->parent->parent)
      continue;
    if (ojama_height) {
      for (int x = 1; x <= 6; x++) {
        for (int y = 1; y <= 13; y++) {
          if (p->field.Get(x, y) == PuyoColor::EMPTY) {
            for (int j = 0; j < ojama_height && y + j <= 13; j++) {
              p->field.Set(x, y + j, PuyoColor::OJAMA);
            }
            break;
          }
        }
      }
    }
    double value;
    if (FLAGS_eval2) {
      value = eval2_->eval(p);
    } else {
      eval_->setIsEmergency(game->p[1].expected_ojama > 15);
      value = eval_->eval(p);
    }
    if (best_value < value) {
      best_value = value;
      best_plan = &plans[i];
    }
  }

  if (!best_plan) {
    if (ojama_height) {
      LOG(INFO) << "I may die due to ojama, but retrying...";
      ojama_height = 0;
      goto retry;
    }
    LOG(WARNING) << "I have no choice";
    return Decision();
  }

  if (is_solo_) {
    msg_ = getEvalString(*best_plan);
  }

  LOG(INFO) << "best_value=" << best_value
            << " score=" << (best_plan ? best_plan->score : -1)
            << " ojama=" << ojama
            << " ojama_height=" << ojama_height
            << ' ' << getEvalString(*best_plan)
            << '\n' << best_plan->field.GetDebugOutput();

  return best_plan->getFirstDecision();
}

string Core::getEvalString(const LP& plan) const {
  if (FLAGS_eval2) {
    return "";
  } else {
    return eval_->getEvalString(plan);
  }
}
