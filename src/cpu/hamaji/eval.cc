#include "eval.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "field.h"
#include "mutex.h"
#include "util.h"

DEFINE_string(params, "params.txt", "");
DEFINE_string(teacher, "kame-decisions.txt", "");

DEFINE_int32(iter, 100, "");
DEFINE_double(delta, 0.000001, "");
DEFINE_double(alpha, 1, "");
DEFINE_double(alpha_decay, 0.99, "");
DEFINE_int32(stages, 1, "");

DEFINE_bool(use_prospective_chains, false, "");
DEFINE_string(template_array, "", "");

enum {
#define DEFINE_PARAM(name, num) name,
#include "params.tab"
#undef DEFINE_PARAM
  TA_SCORE
};

const char* PARAM_NAMES[] = {
#define DEFINE_PARAM(name, num) #name,
#include "params.tab"
#undef DEFINE_PARAM
  "TA_SCORE"
};

static int g_eval_cnt = 0;

static bool g_use_ta;
static int g_ta[13*6][13*6];


struct Eval::Param {
  int id;
  double value;
  string name;
};

Mutex Eval::g_mu;
vector<Eval::Param> Eval::g_params;
vector<int> Eval::g_param_index_offsets;

void Eval::init() {
  MutexLock lock(&g_mu);

  static bool inited = false;
  if (inited)
    return;
  inited = true;

  int offset = 0;
#define DEFINE_PARAM(name, num)                        \
  g_param_index_offsets.push_back(offset);             \
  offset += num;
#include "params.tab"
#undef DEFINE_PARAM
  g_param_index_offsets.push_back(offset);

  FILE* fp = fopen(FLAGS_params.c_str(), "rb");
  CHECK(fp) << FLAGS_params;
  for (int k = 0; k < FLAGS_stages; k++) {
    int j = 0;
    for (size_t i = 0; i < g_param_index_offsets.size() - 1; i++) {
      for (; j < g_param_index_offsets[i + 1]; j++) {
        char buf[256];
        bool eof = fgets(buf, 255, fp) == NULL;
        if (buf[0] == '\n' || buf[0] == '#') {
          j--;
          continue;
        }

        Param param;
        param.id = j - g_param_index_offsets[i];
        param.name = ssprintf("%s-%d", PARAM_NAMES[i], param.id);
        if (eof) {
          if (k > 0) {
            param.value = g_params[j % g_param_index_offsets.back()].value;
          } else {
            param.value = 0;
          }
        } else {
          CHECK_EQ(sscanf(buf, "%lf", &param.value), 1) << buf;
        }
        g_params.push_back(param);
      }
    }
  }
  fclose(fp);

  for (size_t i = 0; i < g_params.size(); i++) {
    const Param& p = g_params[i];
    LOG(INFO) << p.name.c_str() << ' ' << p.value;
  }

  g_use_ta = !FLAGS_template_array.empty();
  if (g_use_ta) {
    fp = fopen(FLAGS_template_array.c_str(), "rb");
    for (int i = 0; i < 13*6; i++) {
      for (int j = 0; j < 13*6; j++) {
        CHECK_EQ(fscanf(fp, "%d", &g_ta[i][j]), 1)
            << "Format error: " << FLAGS_template_array;
      }
    }
    fclose(fp);
  }
}

Eval::Eval() {
  init();
  params_ = g_params;
  param_index_offsets_ = g_param_index_offsets;
}

Eval::~Eval() {
}

int Eval::getParamIndex(int id, int i) {
  return param_index_offsets_[id] + i;
}

double Eval::getConnectionScore(const LF& field,
                                vector<double>* param_scores) {
  double connection_score = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 13; y++) {
      int num_colors[] = {
        0, 0, 0, 0, 0, 0, 0, 0
      };
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
          //printf("y=%d x=%d %d\n", y+dy, x+dx, field.f[y+dy][x+dx] + 1);
          num_colors[(int)field.Get(x+dx, y+dy)]++;
        }
      }
      for (int i = RED; i <= GREEN; i++) {
        // We need this check for 13 danme
        if (num_colors[i] > 4) {
          num_colors[i] = 4;
          //LOG(FATAL) << "x=" << x << " y=" << y << field.GetDebugOutput();
        }
        (*param_scores)[getParamIndex(CONNECTION, num_colors[i])]++;
      }
    }
  }
  return connection_score;
}

void Eval::getHeightScore(const LF& field, vector<double>* param_scores) {
  int heights[6];
  double sum_height = 0;
  for (int x = 0; x < 6; x++) {
    int h = 0;
    for (int y = 1; y <= 13; y++) {
      if (!field.Get(x + 1, y))
        break;
      h++;
    }
    heights[x] = h;
    sum_height += h;
  }

  double avg_height = sum_height / 6;
  double dev_height = 0;
  for (int x = 0; x < 6; x++) {
    double d = heights[x] - avg_height;
    dev_height += d * d;
  }
  dev_height = sqrt(dev_height / 6);
  (*param_scores)[getParamIndex(HEIGHT_DEV, 0)] = dev_height;

  //if (avg_height > 4) {
    double d;
    d = heights[2] - avg_height;
    //if (d > 0)
      (*param_scores)[getParamIndex(HEIGHT_AVG_DIFF, 0)] = d;
    d = heights[3] - avg_height;
    //if (d > 0)
      (*param_scores)[getParamIndex(HEIGHT_AVG_DIFF, 1)] = d;
  //}

  int min_height = 99, max_height = 0;
  for (int x = 0; x < 6; x++) {
    min_height = min(min_height, heights[x]);
    max_height = max(max_height, heights[x]);
  }

  (*param_scores)[getParamIndex(HEIGHT_DIFF, 0)] = max_height - min_height;

  (*param_scores)[getParamIndex(HEIGHT_CORNER_DIFF, 0)] =
    heights[0] - heights[1];
  (*param_scores)[getParamIndex(HEIGHT_CORNER_DIFF, 1)] =
    heights[5] - heights[4];
}

void Eval::getVertical3(const LF& field, vector<double>* param_scores) {
  for (int x = 1; x <= 6; x++) {
    for (int y = 3; y <= 4; y++) {
      char c = field.Get(x, y);
      if (c == EMPTY)
        break;
      if (c >= RED) {
        if (field.Get(x, y-1) == c && field.Get(x, y-2) == c)
          (*param_scores)[getParamIndex(VERTICAL_3, 0)]++;
      }
    }
  }
}

void Eval::fillEvalParamVector(const LF& field,
                               vector<double>* param_scores,
                               int* chain_cnt) {
  g_eval_cnt++;
  param_scores->resize(params_.size());

  int ipc = 0, ucc = 0, vpc = 0;
  int cc = 0;
  if (FLAGS_use_prospective_chains) {
    vector<Chain*> pchains;
    field.getProspectiveChains(&pchains);
    if (!pchains.empty()) {
      cc = pchains[0]->chains;
      vpc = pchains[0]->vanished;
      for (size_t i = 0; i < pchains.size(); i++) {
        if (pchains[i]->score > 840)
          ucc++;
      }
      delete_clear(&pchains);
    }
  } else {
    ipc = 0, ucc = 0, vpc = 0;
    cc = field.getBestChainCount(&ipc, &ucc, &vpc);
  }
  vpc -= cc * 4;
  (*param_scores)[getParamIndex(CHAIN_COUNT, cc)]++;
  (*param_scores)[getParamIndex(USEFUL_CHAIN_COUNT, 0)] = ucc;
  //(*param_scores)[getParamIndex(SIMULTANEOUS_VANISH, 0)] = vpc;
  if (chain_cnt)
    *chain_cnt = cc;

  if (ipc) {
    if (ipc >= 3)
      ipc = 3;
    (*param_scores)[getParamIndex(IGNITION_POINT_PUYO_COUNT, ipc - 1)] = 1;
  }

  getConnectionScore(field, param_scores);

  getHeightScore(field, param_scores);

  //getVertical3(field, param_scores);
}

double Eval::evalFromParamVector(const vector<double>& param_scores) {
#if 0
  int stage = puyo_cnt * FLAGS_stages / 70;
  if (stage >= FLAGS_stages) {
    stage = FLAGS_stages - 1;
  }
  int o = g_param_index_offsets.back() * stage;
#else
  int o = 0;
#endif
  double score = 0;
  for (int i = 0; i < param_index_offsets_.back(); i++) {
    score += param_scores[i] * params_[o + i].value;
  }
  return score;
}

double Eval::eval(LP* plan) {
  vector<double> param_scores(params_.size());
  int chain_cnt = 0;
  fillEvalParamVector(plan->field, &param_scores, &chain_cnt);
  plan->evals.resize(param_index_offsets_.size(), 0);

  size_t j = 0;
  int o = 0;
  plan->chain_cnt = chain_cnt;
  for (int i = 0; i < param_index_offsets_.back(); i++) {
    if (i == param_index_offsets_[j+1]) {
      j++;
    }
    plan->evals[j] += param_scores[i] * params_[o + i].value;
  }
  double r = evalFromParamVector(param_scores);

  int puyo_cnt = plan->field.countPuyo();

  if (puyo_cnt < 20 && plan->chain_cnt)
    r -= 1000;

#if 0
  int color_puyo_cnt = plan->field.countColorPuyo();
  r += (colorPuyoCnt - puyoCnt) * 0.5;
#endif
  int hidden_color_puyo_cnt = 0;
  int ojama_height = plan->field.getOjamaFilmHeight(&hidden_color_puyo_cnt);
  if (hidden_color_puyo_cnt > 10) {
    //LOG(INFO) << ojama_height << " r=" << r;
    r -= ojama_height * 20;
  }

  if (plan->field.Get(3, 11))
    r -= 5;

  if (puyo_cnt < 40) {
    const LP* p = plan;
    while (p->parent) {
      int score = p->score - p->parent->score;
      if (score > 0 && score < 200)
        r -= 1;
      p = p->parent;
    }
    if (p->score > 0 && p->score < 200)
      r -= 1;
  }

  r -= 0.00001 * plan->chigiri_frames;

  if (g_use_ta) {
    double ta_score = 0;
    for (int p1 = 0; p1 < 13*6; p1++) {
      int x1 = p1 % 6 + 1;
      int y1 = p1 / 6 + 1;
      char c1 = plan->field.Get(x1, y1);
      if (c1 < RED)
        continue;
      for (int p2 = p1+1; p2 < 13*6; p2++) {
        int x2 = p2 % 6 + 1;
        int y2 = p2 / 6 + 1;
        char c2 = plan->field.Get(x2, y2);
        if (c2 < RED)
          continue;

        int tav = g_ta[p1][p2];
        if (tav == 0)
          continue;
        if (tav > 0) {
          if (c1 != c2)
            ta_score -= tav * 10;
          else
            ta_score += 1;
        } else if (tav < 0 && c1 == c2) {
          ta_score += tav * 10;
        }
      }
    }
    plan->evals[TA_SCORE] = ta_score;
    r += ta_score;
  }

  return r;
}

const string Eval::getEvalString(const LP& plan) {
  ostringstream oss;
  oss << "chain_cnt=" << plan.chain_cnt;
  for (size_t i = 0; i < plan.evals.size(); i++) {
    oss << ' ';
    oss << PARAM_NAMES[i] << '=' << plan.evals[i];
  }
  return oss.str();
}

// === study ===

struct Eval::Teacher {
  LF problem;
  string next;
  LF answer;
  vector<LP> solutions;
  vector<double> answer_scores;
  vector<vector<double> > solution_scores;
  int puyo_cnt;
};

static void parseField(char* p, LF* field) {
  for (int y = 12; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      int c = *p - '0';
      CHECK_GE(c, 0) << x << "," << y;
      CHECK_LE(c, 5) << x << "," << y;
      field->Set(x, y, "045671"[c] - '0');
      p++;
    }
  }
}

static void parseNext(const char* p, string* next) {
  next->resize(6);
  for (int i = 0; i < 6; i++) {
    int c = p[i] - '0';
    CHECK_GE(c, 0) << i;
    CHECK_LE(c, 5) << i;
    (*next)[i] = "045671"[c] - '0';
  }
}

static double sigmoid(double v) {
  return 1.0f / (1.0f + expf(-v));
}

double Eval::calcParamError(vector<Teacher>& teachers) {
  double sum = 0;
  for (size_t i = 0; i < teachers.size(); i++) {
    Teacher& t = teachers[i];
    double ans = evalFromParamVector(t.answer_scores);
    //LOG(INFO) << "i=" << i << " ans=" << ans;
    for (size_t m = 0; m < t.solutions.size(); m++) {
      double ev =
        evalFromParamVector(t.solution_scores[m]) - ans;
      sum += sigmoid(ev) - 0.5;
    }
  }
  return sum;
}

void Eval::study() {
  FILE* fp = fopen(FLAGS_teacher.c_str(), "rb");
  CHECK(fp) << FLAGS_teacher;
  char buf[1024], pbuf[256], nbuf[6], abuf[256];
  vector<Teacher> teachers;
  int turn = 0;
  bool should_read = true;
  while (fgets(buf, 1023, fp)) {
    if (buf[0] == '=') {
      should_read = true;
      turn = 0;
      continue;
    }
    if (strchr(buf, '5')) {
      should_read = false;
    }
    if (!should_read) {
      // continue;
    }
#if 0
    if (turn >= 20)
      continue;
#endif
    turn++;
    CHECK_EQ(sscanf(buf, "%s%s%s", pbuf, nbuf, abuf), 3) << buf;

    /*
    if (i < 2500)
      continue;
    */
    Teacher t;
    parseField(pbuf, &t.problem);
    parseNext(nbuf, &t.next);
    parseField(abuf, &t.answer);
    int chains, score, frames;
    t.answer.Simulate(&chains, &score, &frames);
    if (chains)
      continue;
    //chains = t.answer.getBestChainCount();
    t.puyo_cnt = t.problem.countPuyo();

    //LOG(INFO) << t.problem.GetDebugOutput();

    fillEvalParamVector(t.answer, &t.answer_scores);

    vector<LP> plans;
    t.problem.FindAvailablePlans(t.next, 1, &plans);
    for (size_t j = 0; j < plans.size(); j++) {
      if (!plans[j].score /* && plans[j].field.getBestChainCount() == chains */) {
        t.solutions.push_back(plans[j]);
      }
    }
    t.solution_scores.resize(t.solutions.size());
    for (size_t j = 0; j < t.solutions.size(); j++) {
      fillEvalParamVector(t.solutions[j].field, &t.solution_scores[j]);
    }
    teachers.push_back(t);
  }

  LOG(INFO) << "# of teachers: " << teachers.size();

  const double kDelta = FLAGS_delta;
  double kAlpha = FLAGS_alpha;
  double base = 0;
  for (int t = 0; t < FLAGS_iter; t++) {
    kAlpha *= FLAGS_alpha_decay;
    base = calcParamError(teachers);
    fprintf(stderr, "base=%f\n", base);

    vector<double> diffs(params_.size());

    for (size_t i = 0; i < params_.size(); i++) {
      params_[i].value += kDelta;
      diffs[i] = calcParamError(teachers) - base;
      params_[i].value -= kDelta;
    }

    for (size_t i = 0; i < params_.size(); i++) {
      params_[i].value -= diffs[i] * kAlpha;
    }
  }

  printf("# techers=%s(%d) base=%f cnt=%d iter=%d delta=%f alpha=%f*%f\n",
         FLAGS_teacher.c_str(), (int)teachers.size(), base, g_eval_cnt,
         FLAGS_iter, FLAGS_delta, FLAGS_alpha, FLAGS_alpha_decay);
  for (size_t i = 0; i < params_.size(); i++) {
    printf("# %s\n", params_[i].name.c_str());
    printf("%f\n", params_[i].value);
  }

  fprintf(stderr, "eval count=%d\n", g_eval_cnt);
}
