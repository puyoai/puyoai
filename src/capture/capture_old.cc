
namespace {

static const char* RC_COLOR_NAMES[] = {
    "EMPTY",
    "OJAMA",
    "ERR",
    "ERR",
    "RED",
    "BLUE",
    "YELLOW",
    "GREEN",
    "PURPLE",
    "VANISHING",
};

static const char* GAME_STATE_NAMES[] = {
  "INIT",
  "TITLE",
  "FINISHED",
  "MODE_SELECT",
  "RUNNING",
};



void emitNext(ostream& oss, const Colors* next) {
  for (int j = 0; j < 6; j++) {
    int p = (int)next[j ^ 1];
    assert(p >= EMPTY);
    assert(p <= GREEN);
    oss << p;
  }
}

}  // anonymous namespace


void Capture::init() {
    mode_ = NONE;
    capture_ticks_ = 0;
    capture_frames_ = 0;
    start_animation_frames_ = 0;

    CLEAR_ARRAY(puyo_);
    finishGame();
    frames_after_next_disappeared_[0] = frames_after_next_disappeared_[1] = INT_MAX;

    grounded_[0] = grounded_[1] = true;
    chain_finished_[0] = chain_finished_[1] = true;
    game_state_ = GAME_INIT;

    puyo_fu_.reset(new PuyoFu());
    if (FLAGS_commentator) {
        commentator_.reset(new Commentator());
    }
}

Capture::Capture() :
    sourceSurface_(nullptr, SDL_FreeSurface)
{
    init();
}

Capture::~Capture()
{
}

void Capture::addSourceFrame(std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> surface)
{
    capture_frames_++;
    start_animation_frames_--;

    sourceSurface_ = std::move(surface);

    if (mode_ == NONE) {
        if (!detectMode()) {
            LOG(ERROR) << "Detecting mode failed";
            return;
        }

#if 0
        // Do we need this?
        if (scr_->scr() && scr_->scr()->w != surf->w) {
            Box b;
            b.sx = 144;
            b.dx = 656;
            b.sy = 40;
            b.dy = 424;
            scr_->setMainBox(surf->w, surf->h, b);
        }
        scr_->setBoundingBox(&bb_[0][0][0]);
#endif
    }

  Uint32 start_ticks = SDL_GetTicks();

  if (commentator_.get())
    commentator_->tick();

  num_vanishing_[0] = num_vanishing_[1] = 0;
  for (int i = 0; i < 2; i++) {
    int pi = player_index(i);
    for (int x = 0; x < 6; x++) {
      for (int y = 0; y < 13; y++) {
        const Box& b = bb_[i][x][y];
        if (b.dx == 0)
          continue;

        RGB rgb;
        int cnt = 0;
        RGB parity_rgb[2];
        int parity_cnt[2] = {0, 0};
        for (int by = b.sy; by <= b.dy; by++) {
          for (int bx = b.sx; bx <= b.dx; bx++) {
              Uint32 c = getpixel(sourceSurface_.get(), bx, by);
            Uint8 r, g, b;
            SDL_GetRGB(c, sourceSurface_->format, &r, &g, &b);
            if (r < 60 && g < 60 && b < 30)
              continue;
            parity_rgb[by%2].r += r;
            parity_rgb[by%2].g += g;
            parity_rgb[by%2].b += b;
            parity_cnt[by%2]++;
            rgb.r += r;
            rgb.g += g;
            rgb.b += b;
            cnt++;
          }
        }

        rgb.div(cnt);
        rgb_[pi][x][y] = rgb;

        if (mode_ == NICO) {
          RealColor rc = toPuyo(rgb_[pi][x][y], x, y);

          int ppi = prev_player_index(i);
          if (puyo_[ppi][x][y] != RC_EMPTY && rc == RC_EMPTY &&
              is_vanishing_[pi][x][y] == 0) {
            puyo_[pi][x][y] = puyo_[ppi][x][y];
            is_vanishing_[pi][x][y] = 1;
          } else {
            puyo_[pi][x][y] = rc;
            is_vanishing_[pi][x][y] = 0;
          }
        } else {
          parity_rgb[0].div(parity_cnt[0]);
          parity_rgb[1].div(parity_cnt[1]);
          HSV parity_hsv0 = parity_rgb[0].toHSV();
          HSV parity_hsv1 = parity_rgb[1].toHSV();

          float phd = hsvDiff2(parity_hsv0, parity_hsv1);
          if (phd > 5000 && y < 12) {
            RealColor o0 = toPuyo(parity_rgb[0], parity_hsv0, x, y);
            RealColor o1 = toPuyo(parity_rgb[1], parity_hsv1, x, y);

            if (o0 != o1 && (o0 == RC_EMPTY || o1 == RC_EMPTY)) {
              is_vanishing_[pi][x][y] = 1;

              if (VERBOSE) {
                float prd = colorDiff2(parity_rgb[0], parity_rgb[1]);
                fprintf(stderr, "parity diff: %f %f x=%d y=%d "
                        "%.1f,%.1f,%.1f (%.1f,%.1f,%.1f) vs "
                        "%.1f,%.1f,%.1f (%.1f,%.1f,%.1f) cnt=%d %s %s\n",
                        phd, prd, x, y,
                        parity_rgb[0].r, parity_rgb[0].g, parity_rgb[0].b,
                        parity_hsv0.h, parity_hsv0.s, parity_hsv0.v,
                        parity_rgb[1].r, parity_rgb[1].g, parity_rgb[1].b,
                        parity_hsv1.h, parity_hsv1.s, parity_hsv1.v,
                        cnt, RC_COLOR_NAMES[o0], RC_COLOR_NAMES[o1]);
              }

              puyo_[pi][x][y] = (o0 != RC_EMPTY) ? o0 : o1;
              continue;
            }
          }

          is_vanishing_[pi][x][y] = 0;

          //fprintf(stderr, "valid cnt=%d\n", cnt);
          RealColor rc = cnt > 20 ? toPuyo(rgb_[pi][x][y], x, y) : RC_EMPTY;
          puyo_[pi][x][y] = rc;
        }
      }
    }

    num_ojama_[i] = 0;
    for (int x = 0; x < 6; x++) {
      for (int y = 11; y >= 0; y--) {
        if (puyo_[pi][x][y] == RC_EMPTY)
          break;
        if (puyo_[pi][x][y] == RC_OJAMA)
          num_ojama_[i]++;
      }
    }

    for (int x = 0; x < 6; x++) {
      for (int y = 11; y >= 0; y--) {
        if (ai_puyo_[i][x][y] == EMPTY)
          break;
        if (is_vanishing_[pi][x][y] == 1) {
          int n = countNumVanishing(pi, x, y);
          if (n > 3)
            num_vanishing_[i] += n;
        }
      }
    }

  }

#if 0
  // A game finished.
  if (!has_puyo) {
    finishGame();
    game_state_ = GAME_FINISHED;
    fprintf(stderr, "game finished\n");
  }
#endif

  int prev_game_state = game_state_;
  calcState();
  if (prev_game_state != game_state_)
    fprintf(stderr, "game state: %s => %s ID=%d\n",
            GAME_STATE_NAMES[prev_game_state], GAME_STATE_NAMES[game_state_],
            capture_frames_);

  maybeUpdateComment();

  if (FLAGS_show_states) {
    dumpStateInfo();
  }
  if (FLAGS_show_messages) {
    for (int i = 0; i < 2; i++) {
      fprintf(stderr, "%dP %s\n", i + 1, getMessageFor(i).c_str());
    }
  }

  Uint32 elapsed = SDL_GetTicks() - start_ticks;
  capture_ticks_ += elapsed;
  //fprintf(stderr, "%u ticks\n", elapsed);
}

void Capture::maybeUpdateComment() {
  for (int pi = 0; pi < 2; pi++) {
    if ((state_[pi] & (STATE_WNEXT_APPEARED |
                       STATE_YOU_GROUNDED |
                       STATE_CHAIN_DONE)) == 0) {
      continue;
    }

    FieldWithColorSequence f;
    for (int x = 0; x < 6; x++) {
      for (int y = 11; y >= 0; y--) {
        char c = ai_puyo_[pi][x][y];
        if (c == EMPTY)
          break;
        f.Set(x+1, 12-y, c);
      }
    }

    string seq;
    for (int i = 0; i < 6; i++) {
      char c = ai_next_[pi][i ^ 1];
      if (!c)
        break;
      seq += '0' + c;
    }
#if 0
    if (seq.size() != 4 && seq.size() != 6)
      continue;
#endif

    for (Observer* observer : observers_)
        observer->onUpdate();

    f.SetColorSequence(seq);

    if (commentator_.get()) {
      commentator_->setField(pi, f, (state_[pi] & STATE_YOU_GROUNDED) != 0);
    }

    if (puyo_fu_.get() && seq.size() >= 4) {
      puyo_fu_->setField(pi, f, state_[pi], capture_frames_);
    }
  }
}

int Capture::countNumVanishing(int pi, int x, int y) {
  if (x < 0 || y < 0 || x >= 6 || y >= 12)
    return 0;

  if (is_vanishing_[pi][x][y] != 1)
    return 0;

  is_vanishing_[pi][x][y] = 2;
  int n = 1;
  n += countNumVanishing(pi, x+1, y);
  n += countNumVanishing(pi, x-1, y);
  n += countNumVanishing(pi, x, y+1);
  n += countNumVanishing(pi, x, y-1);
  return n;
}

void Capture::finishGame() {
  winner_ = 0;
  memset(color_map_, -1, sizeof(color_map_));
  CLEAR_ARRAY(ai_puyo_);
  CLEAR_ARRAY(ai_next_);
  CLEAR_ARRAY(ai_num_ojama_);
  CLEAR_ARRAY(has_next_);
  CLEAR_ARRAY(has_wnext_);
  CLEAR_ARRAY(state_);
  grounded_[0] = grounded_[1] = true;
}

bool Capture::isTitle() const {
  int pi = player_index(0);
  RGB rgb = rgb_[pi][4][12];
  HSV hsv = rgb.toHSV();

  bool is_title = puyo_[pi][4][12] != RC_YELLOW;
  if (is_title) {
    if (VERBOSE)
      fprintf(stderr, "title! %f %f %f %f %f %f => %s\n",
              rgb.r, rgb.g, rgb.b, hsv.h, hsv.s, hsv.v,
              RC_COLOR_NAMES[puyo_[pi][4][12]]);
  }

  return is_title;
}

bool Capture::isModeSelect() const {
  int pi = player_index(1);
  RGB rgb = rgb_[pi][4][12];
  HSV hsv = rgb.toHSV();

  bool is_mode_select;
  is_mode_select = rgb.r > 220 && rgb.g > 220 && rgb.b > 210;
  if (mode_ == NICO && rgb.r > 200 && rgb.g > 200 && rgb.b > 170) {
    is_mode_select = true;
  }

  if (is_mode_select) {
    if (VERBOSE)
      fprintf(stderr, "mode select! %f %f %f %f %f %f => %s\n",
              rgb.r, rgb.g, rgb.b, hsv.h, hsv.s, hsv.v,
              RC_COLOR_NAMES[puyo_[pi][5][12]]);
  }

  return is_mode_select;
}

void Capture::updateWinner() {
  int v = 0;
  for (int pi = 0; pi < 2; pi++) {
    if (puyo_[pi][5][12] != RC_YELLOW &&
        puyo_[pi+2][5][12] != RC_YELLOW)
      v += pi + 1;
  }
  winner_ = v;
}

void Capture::calcState() {
  if (isTitle()) {
    game_state_ = GAME_TITLE;
    return;
  }

  if (isModeSelect()) {
    game_state_ = GAME_MODE_SELECT;
    finishGame();
    return;
  }

  // Update game state 1 frame afer winner is filled so we have
  // one frame where we are running but a winner is decided.
  if (winner_) {
    if (commentator_.get()) {
      commentator_->reset();
    }

    if (!FLAGS_puyofu_field_transition_log.empty() && !puyo_fu_->empty()) {
      for (int pi = 0; pi < 2; pi++) {
        string out = FLAGS_puyofu_field_transition_log;
        if (pi)
          out += "_2p";
        out += ".txt";
        FILE* fp = fopen(out.c_str(), "a");
        puyo_fu_->emitFieldTransitionLog(fp, pi);
        fprintf(fp, "=== end ===\n");
        fclose(fp);
      }
    }
    puyo_fu_.reset(new PuyoFu());

    game_state_ = GAME_FINISHED;
    updateWinner();
    return;
  }
  updateWinner();

  if (game_state_ == GAME_MODE_SELECT) {
    game_state_ = GAME_RUNNING;
    start_animation_frames_ = 20;
    for (int i = 0; i < 2; i++) {
      updateAINext(i);
      has_next_[i] = true;
      has_wnext_[i] = true;
      setGrounded(i);
    }
  }

  // If you grouned in the previous frame, we'll move nexts.
  for (int i = 0; i < 2; i++) {
    if ((state_[i] & STATE_YOU_GROUNDED) != 0) {
      moveNexts(i);
    }
  }

  for (int i = 0; i < 2; i++) {
    state_[i] = STATE_NONE;
  }

  for (int i = 0; i < 2; i++) {
    int pi = player_index(i);
    if (has_next_[i]) {
      if (puyo_[pi][0][12] == RC_EMPTY && puyo_[pi][1][12] == RC_EMPTY) {
        has_next_[i] = false;
        // TODO(hamaji): Adjust this value!
        frames_after_next_disappeared_[i] = 2;

        if (!grounded_[i])
          setGrounded(i);
        if (!chain_finished_[i])
          state_[i] |= STATE_CHAIN_DONE;

        grounded_[i] = false;
        chain_finished_[i] = false;

        // TODO(hamaji): For the first puyo...
        if (ai_next_[i][0] == EMPTY && ai_next_[i][1] == EMPTY) {
          moveNexts(i);
        }

        updateAIField(i);
      }
    } else {
      if (puyo_[pi][0][12] >= RC_RED && puyo_[pi][1][12] >= RC_RED) {
        if (!isNextStable(i, 0) || !isNextStable(i, 1))
          continue;

#if 0
        float cd1 = colorDiff2(rgb_[pi][2][12], rgb_[ppi][2][12]);
        float cd2 = colorDiff2(rgb_[pi][3][12], rgb_[ppi][3][12]);
        fprintf(stderr, "cd1=%f cd2=%f cf=%d pi=%d ppi=%d\n",
                cd1,cd2,capture_frames_,pi,ppi);
        fprintf(stderr, "rgb=%.1f,%.1f,%.1f prgb=%.1f,%.1f,%.1f\n",
                rgb_[pi][2][12].r, rgb_[pi][2][12].g, rgb_[pi][2][12].b,
                rgb_[ppi][2][12].r, rgb_[ppi][2][12].g, rgb_[ppi][2][12].b);
#endif

        has_next_[i] = true;
      }
    }
  }

#if 0
  for (int i = 0; i < 2; i++) {
    int pi = player_index(i);
    for (int j = 0; j < 4; j++) {
      if (j < 2) {
        if (!has_next_[i])
          continue;
      } else {
        if (!has_wnext_[i])
          continue;
      }

      RealColor rc = puyo_[pi][j][12];
      if (rc < RC_RED)
        continue;
      // Ignore unallocated color.
      if (color_map_[rc - RC_RED] == -1)
        continue;

      Colors c = getAIColor(rc);
      if (ai_next_[i][j+2] == c) {
        continue;
      }
      if (j < 2 && ai_next_[i][j+2] < RED) {
        continue;
      }

      fprintf(stderr,
              "COME! i=%d j=%d %d vs %d(%d) "
              "%d %d %d %d %d %d %d %d\n",
              i, j, ai_next_[i][j+2], c, rc,
              puyo_[pi][0][12], puyo_[pi][1][12],
              puyo_[pi][2][12], puyo_[pi][3][12],
              isNextStable(i, 0), isNextStable(i, 1),
              isNextStable(i, 2), isNextStable(i, 3));

      if (puyo_[pi][0][12] < RC_RED || puyo_[pi][1][12] < RC_RED ||
          puyo_[pi][1][12] < RC_RED || puyo_[pi][3][12] < RC_RED ||
          !isNextStable(i, 0) || !isNextStable(i, 1) ||
          !isNextStable(i, 2) || !isNextStable(i, 3))
        break;

      // Unfortunately, this can happen because video capture device
      // misses a few frames. Wait until the all next puyos becomes
      // stable, update the info, and send appropriate info to AIs.
      fprintf(stderr, "We missed (w)next vanish, force updating...\n");
      //LOG(ERROR) << "We missed (w)next vanish, force updating...";
      Colors orig_next[6];
      orig_next[2] = ai_next_[i][2];
      orig_next[3] = ai_next_[i][3];
      orig_next[4] = ai_next_[i][4];
      orig_next[5] = ai_next_[i][5];
      updateAINext(i);

      has_next_[i] = true;
      has_wnext_[i] = true;
      if (orig_next[4] != ai_next_[i][4] || orig_next[5] != ai_next_[i][5]) {
        handleWnextArrival(i);
      }
      break;
    }
  }
#endif

  for (int i = 0; i < 2; i++) {
    if (--frames_after_next_disappeared_[i] <= 0) {
      state_[i] |= STATE_YOU_CAN_PLAY;
    }
  }

  for (int i = 0; i < 2; i++) {
    int pi = player_index(i);
    if (has_wnext_[i]) {
      if (puyo_[pi][2][12] == RC_EMPTY && puyo_[pi][3][12] == RC_EMPTY) {
        has_wnext_[i] = false;
      }
    } else {
      if (puyo_[pi][2][12] >= RC_RED && puyo_[pi][3][12] >= RC_RED) {
        if (!isNextStable(i, 2) || !isNextStable(i, 3))
          continue;

#if 0
        float cd1 = colorDiff2(rgb_[pi][2][12], rgb_[ppi][2][12]);
        float cd2 = colorDiff2(rgb_[pi][3][12], rgb_[ppi][3][12]);
        fprintf(stderr, "wnext color diff: %d %f %f\n", i, cd1, cd2);
        if (cd1 > 100 || cd2 > 100)
          continue;
#endif

        handleWnextArrival(i);
      }
    }
  }

  for (int i = 0; i < 2; i++) {
#if 0
    fprintf(stderr, "i=%d grounded=%d num_vanishing=%d state=%d "
            "frames_after_next_disappeared=%d\n",
            i, grounded_[i], num_vanishing_[i], state_[i],
            frames_after_next_disappeared_[i]);
#endif
    if (!grounded_[i] && num_vanishing_[i] >= 4 && (state_[i] & STATE_YOU_CAN_PLAY)) {
      state_[i] &= ~STATE_YOU_CAN_PLAY;
      frames_after_next_disappeared_[i] = INT_MAX;
      setGrounded(i);
      grounded_[i] = true;
      updateAIField(i);
    }
  }

  for (int i = 0; i < 2; i++) {
    if (!grounded_[i] && num_ojama_[i] > ai_num_ojama_[i]) {
      if (!isStableField(i)) {
        continue;
      }

      state_[i] &= ~STATE_YOU_CAN_PLAY;
      frames_after_next_disappeared_[i] = INT_MAX;
      setGrounded(i);
      grounded_[i] = true;
      updateAIField(i);
    }
  }

  if (VERBOSE)
  fprintf(stderr, "game_state=%d state0=%d ndisf=%d gnd=%d hn=%d hwn=%d\n",
          game_state_,
          state_[0], frames_after_next_disappeared_[0], grounded_[0],
          has_next_[0], has_wnext_[0]);

  if ((state_[0] & ~STATE_YOU_CAN_PLAY) ||
      (state_[1] & ~STATE_YOU_CAN_PLAY) ||
      frames_after_next_disappeared_[0] == 0 ||
      frames_after_next_disappeared_[1] == 0) {
    fprintf(stderr, "%s %s\n", getMessageFor(0).c_str(), frame_info_.c_str());
  }
}

bool Capture::isStableField(int i) const {
  return !memcmp(puyo_[i], puyo_[i | 2], sizeof(puyo_[i]));
}

void Capture::handleWnextArrival(int i) {
  has_wnext_[i] = true;
  state_[i] |= STATE_WNEXT_APPEARED;

  updateAINext(i);
  if (game_state_ == GAME_FINISHED) {
    updateAIField(i);
  }
}

std::string Capture::getMessageFor(int pi) {
  int state = 0;
  for (int i = 0; i < 2; i++) {
    int s = (i != pi);
    state |= state_[i] << s;
  }

  ostringstream oss;
  oss << "ID=" << capture_frames_
      << " STATE=" << state;
  for (int i = 0; i < 2; i++) {
    int ci = (i != pi);

    oss << (i ? " OF=" : " YF=");
    for (int y = 0; y < 12; y++) {
      for (int x = 0; x < 6; x++) {
        int p = (int)ai_puyo_[ci][x][y];
        assert(p >= EMPTY);
        assert(p <= GREEN);
        oss << p;
      }
    }

    oss << (i ? " OP=" : " YP=");
    emitNext(oss, ai_next_[ci]);
  }

  // TODO(hamaji): Handle this if possible.
  oss << " YX=3 YY=12 YR=0 OX=3 OY=12 OR=0";
  oss << " YO=0 OO=0 YS=0 OS=0";
  if (winner_) {
    int w = winner_ - 1;
    int e = (w == !pi) - (w == pi);
    oss << " END=" << e;
  }

  return oss.str();
}

void Capture::setAIColor(RealColor rc, Colors c) {
  color_map_[rc - RC_RED] = c;
  int cnt = 0;
  for (int i = 0; i < 5; i++) {
    if (color_map_[i] != -1)
      cnt++;
  }

  if (commentator_.get()) {
    commentator_->setColorMap(rc, c);
  }

  if (cnt >= 0) {
    fprintf(stderr, "%d color map:", cnt);
    for (int i = 0; i < 5; i++) {
      fprintf(stderr, " %d", color_map_[i]);
    }
    fprintf(stderr, "\n");
  }
}

Colors Capture::getAIColor(RealColor rc, bool will_allocate) {
  bool retried = false;
  retry:
  if (rc < RC_RED)
    return (Colors)rc;
  int i = rc - RC_RED;
  if (color_map_[i] == -1) {
    if (!will_allocate)
      return EMPTY;

    if (rc != RC_PURPLE) {
      bool is_used = false;
      for (int j = 0; j < 5; j++) {
        if (color_map_[j] == (Colors)rc) {
          is_used = true;
          break;
        }
      }

      if (!is_used) {
        setAIColor(rc, (Colors)rc);
      }
    }

    if (color_map_[i] == -1) {
      for (int j = RED; j < RED + 4; j++) {
        bool can_use = true;
        for (int k = 0; k < 5; k++) {
          if (color_map_[k] == (Colors)j) {
            can_use = false;
            break;
          }
        }
        if (can_use) {
          setAIColor(rc, (Colors)j);
          break;
        }
      }

      if (color_map_[i] == -1) {
        fprintf(stderr, "TERRIBLE: 5th color appeared!\n");
        if (retried) {
          fprintf(stderr, "Giving up...\n");
          setAIColor(rc, EMPTY);
        } else {
          fprintf(stderr, "Resetting color map...\n");
          memset(color_map_, -1, sizeof(color_map_));
          retried = true;
          goto retry;
        }
      }
    }

    assert(color_map_[i] != -1);
  }
  return color_map_[i];
}

void Capture::updateAIField(int i) {
  game_state_ = GAME_RUNNING;

  // Do not update AI field while start animation is moving.
  if (start_animation_frames_ > 0)
    return;

  memset(ai_puyo_[i], 0, sizeof(ai_puyo_[i]));
  ai_num_ojama_[i] = 0;

  int pi = player_index(i);
  for (int x = 0; x < 6; x++) {
    for (int y = 11; y >= 0; y--) {
      RealColor rc = puyo_[pi][x][y];
      if (rc == RC_EMPTY)
        break;
      if (rc == RC_OJAMA)
        ai_num_ojama_[i]++;
      Colors c = getAIColor(rc);
      ai_puyo_[i][x][y] = c;
    }
  }
}

void Capture::updateAINext(int i) {
  int pi = player_index(i);
  for (int j = 0; j < 4; j++) {
    RealColor rc = puyo_[pi][j][12];
    bool will_allocate = true;
    if (j >= 3 && mode_ == NICO)
      will_allocate = false;
    Colors c = getAIColor(rc, will_allocate);
    ai_next_[i][j+2] = c;
  }
}

static bool isEdgeOfGame(const Capture::HSV& hsv) {
  return (hsv.h > 25 && hsv.h < 45 &&
          hsv.s > 0.4 && hsv.s < 0.7 &&
          hsv.v > 80);
}

bool Capture::detectMode()
{
    CLEAR_ARRAY(bb_);

    // TODO(mayah): sourceSurface_ might be nullptr here?
    // Or there is no such case?

  int ox = 0;
  int oy = 0;
  int w = sourceSurface_->w;
  int h = sourceSurface_->h;

  if ((w == 384 && h == 288) || (w == 720 && h == 480)) {
    mode_ = VCA;

    for (int i = 0; i < 2; i++) {
      Box b;
      for (int x = 0; x < 6; x++) {
        b.sx = 40.0 + 17.0 * (x + i * 12) + i;
        b.dx = 40.0 + 17.0 * (x + i * 12) + i + 12.0;

        for (int y = 0; y < 13; y++) {
          b.sy = 34.0 + 19.0 * y;
          b.dy = 34.0 + 19.0 * y + 12.0;

          if (y == 12) {
            // Game over detection.
            if (x == 0)
              bb_[i][5][y] = b;
          } else {
            bb_[i][x][y] = b;
          }
        }
      }

      bb_[i][0][12].sx = 39.0 + 17.0 * (7 + i * 3) + i * 2;
      bb_[i][0][12].dx = 39.0 + 17.0 * (7 + i * 3) + i * 2 + 12.0;
      bb_[i][0][12].sy = 32.0 + 19.0 * 2;
      bb_[i][0][12].dy = 32.0 + 19.0 * 2 + 12.0;

      bb_[i][1][12].sx = 39.0 + 17.0 * (7 + i * 3) + i * 2;
      bb_[i][1][12].dx = 39.0 + 17.0 * (7 + i * 3) + i * 2 + 12.0;
      bb_[i][1][12].sy = 32.0 + 19.0 * 3;
      bb_[i][1][12].dy = 32.0 + 19.0 * 3 + 12.0;

      bb_[i][2][12].sx = 41.0 + 17.0 * (8 + i) + i * 7.0;
      bb_[i][2][12].dx = 41.0 + 17.0 * (8 + i) + i * 7.0 + 5.0;
      bb_[i][2][12].sy = 32.0 + 19.0 * 3;
      bb_[i][2][12].dy = 32.0 + 19.0 * 3 + 9.0;

      bb_[i][3][12].sx = 41.0 + 17.0 * (8 + i) + i * 7.0;
      bb_[i][3][12].dx = 41.0 + 17.0 * (8 + i) + i * 7.0 + 5.0;
      bb_[i][3][12].sy = 32.0 + 19.0 * 4;
      bb_[i][3][12].dy = 32.0 + 19.0 * 4 + 9.0;
    }

    // Title detection.
    bb_[0][4][12].sx = 21;
    bb_[0][4][12].dx = 40;
    bb_[0][4][12].sy = 11;
    bb_[0][4][12].dy = 30;

    // Mode select detection.
    bb_[1][4][12].sx = 68;
    bb_[1][4][12].dx = 90;
    bb_[1][4][12].sy = 70;
    bb_[1][4][12].dy = 70;

    return true;
  } else if ((w == 512 && h == 384) || mode_ == NICO) {
    mode_ = NICO;

    float bw = (float)w / 20;
    float bh = (float)h / 14;

    for (int x = 0; x < 20; x++) {
      for (int y = 0; y < 14; y++) {
        if (x < 1 || y < 1 || x > 18)
          continue;
        int di = x > 9 ? 1 : 0;
        int dx = x - 1;
        int dy = y - 1;

        if (y == 13) {
          // Game over detection.
          if (x != 6 && x != 13)
            continue;
          dx = 5;
        } else if (x <= 6) {
          dx = x - 1;
        } else if (x == 7) {
          continue;
        } else if (x == 8 || x == 11) {
          if (y == 3)
            dx = 0;
          else if (y == 4)
            dx = 1;
          else
            continue;
          dy = 12;
        } else if (x == 9 || x == 10) {
          if (y == 4)
            dx = 2;
          else if (y == 5)
            dx = 3;
          else
            continue;
          dy = 12;
        } else if (x == 12) {
          continue;
        } else {
          dx = x - 13;
        }
        Box* bb = &bb_[di][dx][dy];

        int px = (int)ceil(x * bw);
        int py = (int)ceil(y * bh);

        bb->sx = px + 2;
        bb->sy = py + 2;
        bb->dx = px + (int)bw - 2;
        bb->dy = py + (int)bh - 2;

        if (x == 9) {
          bb->sx = px + 6;
          bb->dx = px + 12;
          bb->dy -= 5;
        } else if (x == 10) {
          bb->sx = px + 13;
          bb->dx = px + 18;
          bb->dy -= 5;
        } else if (x == 11) {
          bb->sx = px + 6;
          bb->dx = px + (int)bw;
        }
      }
    }

    // Title detection.
    bb_[0][4][12].sx = 5;
    bb_[0][4][12].dx = 35;
    bb_[0][4][12].sy = 5;
    bb_[0][4][12].dy = 35;

    // Mode select detection.
    bb_[1][4][12].sx = 85;
    bb_[1][4][12].dx = 100;
    bb_[1][4][12].sy = 86;
    bb_[1][4][12].dy = 89;

#if 0
    // TODO(mayah): What's this?
    if ((surf_->w == 624 || surf_->w == 640) && surf_->h == 360) {
      for (int i = 0; i < 2; i++) {
        for (int x = 0; x < 6; x++) {
          for (int y = 0; y < 13; y++) {
            const Box& b = bb_[i][x][y];
            if (b.dx == 0)
              continue;
            bb_[i][x][y].sx += ox;
            bb_[i][x][y].dx += ox;
            bb_[i][x][y].sy += oy;
            bb_[i][x][y].dy += oy;
          }
        }
      }
    }
#endif

    return true;
  }
  assert(false);
  return false;
}

void Capture::draw(Screen* screen)
{
    SDL_Surface* screenSurface = screen()->surface();
    SDL_BlitSurface(sourceSurface_.get(), NULL, screenSurface, NULL);

    if (FLAGS_draw_color_detection) {
        SDL_LockSurface(screenSurface);

        Uint32 colors[RC_END];
        colors[RC_EMPTY] = SDL_MapRGB(screenSurface->format, 0, 0, 0);
        colors[RC_OJAMA] = SDL_MapRGB(screenSurface->format, 0, 255, 255);
        colors[RC_RED] = SDL_MapRGB(screenSurface->format, 255, 0, 0);
        colors[RC_BLUE] = SDL_MapRGB(screenSurface->format, 0, 0, 255);
        colors[RC_YELLOW] = SDL_MapRGB(screenSurface->format, 255, 255, 0);
        colors[RC_GREEN] = SDL_MapRGB(screenSurface->format, 0, 255, 0);
        colors[RC_PURPLE] = SDL_MapRGB(screenSurface->format, 255, 0, 255);
        colors[RC_VANISHING] = SDL_MapRGB(screenSurface->format, 255, 255, 255);

        for (int i = 0; i < 2; i++) {
            int pi = player_index(i);
            for (int x = 0; x < 6; x++) {
                for (int y = 0; y < 13; y++) {
                    const Box& b = bb_[i][x][y];
                    if (b.dx == 0)
                        continue;

                    Uint32 col = colors[puyo_[pi][x][y]];
                    Uint32 ucol = col;
                    if (is_vanishing_[pi][x][y]) {
                        ucol = colors[RC_VANISHING];
                    }
                    Colors dpuyo = ai_puyo_[i][x][y];
                    if (y == 12 && x < 4) {
                        dpuyo = ai_next_[i][x+2];
                    }
                    Uint32 dcol = colors[dpuyo];

                    for (int j = b.sx; j <= b.dx; j++) {
                        putpixel(screenSurface, j, b.sy, ucol);
                        putpixel(screenSurface, j, b.dy, dcol);
                    }
                    for (int j = b.sy; j <= b.dy; j++) {
                        putpixel(screenSurface, b.sx, j, col);
                        putpixel(screenSurface, b.dx, j, col);
                    }
                }
            }
        }

        SDL_UnlockSurface(screenSurface);
    }

    screen()->drawFPS();
    mainWindow_->renderScreen();
}

Colors Capture::getColor(int i, int x, int y) const {
  return ai_puyo_[i][x][y];
}

Colors Capture::getNext(int i, int j) const {
  return ai_next_[i][j];
}

Capture::RealColor Capture::getRealColor(int i, int x, int y) const {
  int pi = player_index(i);
  return puyo_[pi][x][y];
}

bool Capture::isVanishing(int i, int x, int y) const {
  int pi = player_index(i);
  return is_vanishing_[pi][x][y];
}

int Capture::numVanishing() const {
  return num_vanishing_[0] + num_vanishing_[1];
}

void Capture::setState(int i, unsigned int s) {
  if ((s & STATE_YOU_CAN_PLAY)) {
    frames_after_next_disappeared_[i] = 0;
    grounded_[i] = false;
  }
  state_[i] = s;
}

void Capture::setGrounded(int i) {
  state_[i] |= STATE_YOU_GROUNDED;
}

void Capture::moveNexts(int i) {
  for (int j = 0; j < 4; j++) {
    ai_next_[i][j] = ai_next_[i][j+2];
  }
  ai_next_[i][4] = ai_next_[i][5] = EMPTY;
}

void Capture::dumpStateInfo() const {
  ostringstream oss;
  if (!frame_info_.empty())
    oss << frame_info_ << '\n';
  oss << "game_state=" << game_state_
      << " start_animation_frames=" << start_animation_frames_
      << '\n';
  for (int i = 0; i < 2; i++) {
    oss << (i+1) << "P state:"
        << " state=" << GetStateString(state_[i])
        << " has_next=" << has_next_[i]
        << " has_wnext=" << has_wnext_[i]
        << " fand=" << frames_after_next_disappeared_[i]
        << " grounded=" << grounded_[i]
        << " chain_fin=" << chain_finished_[i]
        << " vanishing=" << num_vanishing_[i]
        << " next=";
    emitNext(oss, ai_next_[i]);
    oss << '\n';
  }
  fprintf(stderr, "%s", oss.str().c_str());
}

float Capture::calcColorDiff2(int i, int x, int y) const {
  int pi = player_index(i);
  int ppi = prev_player_index(i);
  return colorDiff2(rgb_[pi][x][y], rgb_[ppi][x][y]);
}

float Capture::calcNextColorDiff2(int i, int x) const {
  return calcColorDiff2(i, x, 12);
}

bool Capture::isStable(int i, int x, int y) const {
  return calcColorDiff2(i, x, y) <= 100;
}

bool Capture::isNextStable(int i, int x) const {
  return isStable(i, x, 12);
}

void Capture::setAIMessage(int pi, const string& msg) {
  ai_msg_[pi] = msg;
  if (commentator_.get()) {
    commentator_->setAIMessage(pi, msg);
  }
}
