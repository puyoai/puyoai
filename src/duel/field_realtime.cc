#include "duel/field_realtime.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/constant.h"
#include "core/ctrl.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "duel/frame_context.h"

using namespace std;

DEFINE_bool(delay_wnext, true, "Delay wnext appear");

// TODO(mayah): Why STATE_CHIGIRI is necessary? Can we use STATE_DROP instead?
// Maybe dropping puyo in CHIGIRI state is slower than in DROP?

// STATE_LEVEL_SELECT
//  v
// STATE_USER <-------+
//  v                 |
// STATE_CHIGIRI      |
//  v                 |
// STATE_DROP <----+  |
//  v              |  |
// STATE_VANISH ---+  |
//  v                 |
// STATE_OJAMA ------ +

FieldRealtime::FieldRealtime(int playerId, const KumipuyoSeq& seq) :
    playerId_(playerId)
{
    // Since we don't use the first kumipuyo, we need to put EMPTY/EMPTY.
    vector<Kumipuyo> kps;
    kps.push_back(Kumipuyo(PuyoColor::EMPTY, PuyoColor::EMPTY));
    kps.insert(kps.end(), seq.underlyingData().begin(), seq.underlyingData().end());
    kumipuyoSeq_ = KumipuyoSeq(kps);

    Init();
}

void FieldRealtime::Init()
{
    userState_.playable = false;
    sleepFor_ = 30;

    frames_for_free_fall_ = 0;
    ojama_position_ = vector<int>(6, 0);
    ojama_dropping_ = false;
    current_chains_ = 1;
    quickturn_ = 0;
    dropped_rows_ = 0;
    delayFramesWNextAppear_ = 0;
    sent_wnext_appeared_ = false;
    drop_animation_ = false;
}

bool FieldRealtime::TryChigiri()
{
    if (Chigiri()) {
        dropped_rows_++;
        sleepFor_ =
            (dropped_rows_ == 1) ? FRAMES_CHIGIRI_1_LINE_1 :
            (dropped_rows_ == 2) ? FRAMES_CHIGIRI_1_LINE_2 :
            FRAMES_CHIGIRI_1_LINE_3;

        drop_animation_ = true;
        return true;
    } else {
        dropped_rows_ = 0;
        simulationState_ = STATE_VANISH;
        if (drop_animation_) {
            sleepFor_ = FRAMES_AFTER_CHIGIRI;
            drop_animation_ = false;
        } else {
            sleepFor_ = FRAMES_AFTER_NO_CHIGIRI;
        }
        return false;
    }
}

bool FieldRealtime::TryVanish(FrameContext* context)
{
    // TODO(mayah): field_ looks inconsistent in some reason.
    // Let's recalculate the height.
    for (int x = 1; x <= CoreField::WIDTH; ++x)
        field_.recalcHeightOn(x);

    int score = field_.vanishOnly(current_chains_);
    if (score > 0) {
        current_chains_++;
        score_ += score;
        if (is_zenkesi_) {
            score_ += ZENKESHI_BONUS;
            is_zenkesi_ = false;
        }
        simulationState_ = STATE_DROP;
        sleepFor_ = FRAMES_VANISH_ANIMATION;

        // Set Yokoku Ojama.
        if ((score_ - consumed_score_ >= SCORE_FOR_OJAMA) && (current_chains_ > 1)) {
            int attack_ojama = (score_ - consumed_score_) / SCORE_FOR_OJAMA;
            if (context)
                context->sendOjama(attack_ojama);
            consumed_score_ = score_ / SCORE_FOR_OJAMA * SCORE_FOR_OJAMA;
        }
        return true;
    } else {
        sleepFor_ = FRAMES_AFTER_VANISH;
        FinishChain(context);
        return false;
    }
}

void FieldRealtime::FinishChain(FrameContext* context)
{
    isDead_ = (field_.color(3, 12) != PuyoColor::EMPTY);
    if (!is_zenkesi_) {
        is_zenkesi_ = true;
        for (int i = 1; i <= 6; i++) {
            if (field_.color(i, 1) != PuyoColor::EMPTY) {
                is_zenkesi_ = false;
            }
        }
    }

    if (context)
        context->commitOjama();
    if (numFixedOjama() > 0) {
        simulationState_ = STATE_OJAMA;
    } else {
        PrepareNextPuyo();
    }
    current_chains_ = 1;
    userState_.chainFinished = true;
}

bool FieldRealtime::TryDrop(FrameContext* context)
{
    if (Drop1line()) {
        sleepFor_ = FRAMES_DROP_1_LINE;
        drop_animation_ = true;
        return true;
    } else {
        if (drop_animation_) {
            sleepFor_ = FRAMES_AFTER_DROP;
            drop_animation_ = false;
            simulationState_ = STATE_VANISH;
        } else {
            FinishChain(context);
            sleepFor_ = FRAMES_AFTER_NO_DROP;
        }
        return false;
    }
}

bool FieldRealtime::TryOjama()
{
    if (!ojama_dropping_) {
        ojama_position_ = determineColumnOjamaAmount();
        for (int i = 0; i < 6; i++) {
            if (ojama_position_[i]) {
                ojama_dropping_ = true;
            }
        }
    }

    if (ojama_dropping_) {
        for (int i = 0; i < 6; i++) {
            if (ojama_position_[i] > 0) {
                if (field_.color(i + 1, 13) == PuyoColor::EMPTY) {
                    field_.setPuyoAndHeight(i + 1, 13, PuyoColor::OJAMA);
                }
                ojama_position_[i]--;
            }
        }
    }
    if (Drop1line()) {
        dropped_rows_++;
        sleepFor_ =
            (dropped_rows_ == 1) ? FRAMES_CHIGIRI_1_LINE_1 :
            (dropped_rows_ == 2) ? FRAMES_CHIGIRI_1_LINE_2 :
            FRAMES_CHIGIRI_1_LINE_3;

        return true;
    } else {
        dropped_rows_ = 0;
        ojama_dropping_ = false;
        isDead_ = (field_.color(3, 12) != PuyoColor::EMPTY);
        sleepFor_ = FRAMES_AFTER_DROP;
        userState_.ojamaDropped = true;
        PrepareNextPuyo();
        return false;
    }
}

// Returns true if a key input is accepted.
bool FieldRealtime::playOneFrame(Key key, FrameContext* context)
{
    userState_.clear();

    if (quickturn_ > 0) {
        quickturn_--;
    }

    if (simulationState_ == STATE_USER) {
        frames_for_free_fall_++;
    }

    if (delayFramesWNextAppear_ > 0) {
        --delayFramesWNextAppear_;
    }

    if (delayFramesWNextAppear_ == 0 && !sent_wnext_appeared_) {
        userState_.wnextAppeared = true;
        sent_wnext_appeared_ = true;
    }

    // Loop until some functionality consumes this frame.
    while (true) {
        if (sleepFor_ > 0) {
            sleepFor_--;
            // Player can send a command in the next frame.
            if (simulationState_ == STATE_USER && sleepFor_ == 0) {
                userState_.playable = true;
            }
            return false;
        }

        switch (simulationState_) {
        case STATE_LEVEL_SELECT:
            simulationState_ = STATE_USER;
            PrepareNextPuyo();
            userState_.playable = true;
            continue;
        case STATE_CHIGIRI:
            if (TryChigiri())
                return false;
            continue;
        case STATE_VANISH:
            if (TryVanish(context))
                return false;
            continue;
        case STATE_DROP:
            if (TryDrop(context))
                return false;
            continue;
        case STATE_OJAMA:
            if (TryOjama())
                return false;
            continue;
        case STATE_USER: {
            bool accepted = true;
            bool grounded = PlayInternal(key, &accepted);
            if (!grounded) {
                if (accepted) {
                    sleepFor_ = FRAMES_AFTER_USER_INTERACTION;
                }
                if (frames_for_free_fall_ >= FRAMES_FREE_FALL) {
                    bool dummy;
                    grounded = PlayInternal(KEY_DOWN, &dummy);
                }
            }

            if (grounded) {
                chigiri_x_ = -1;
                chigiri_y_ = -1;
                simulationState_ = STATE_CHIGIRI;
                userState_.grounded = true;
            }

            if (key == KEY_DOWN && accepted) {
                score_++;
            }
            return accepted;
        }
        }
    }  // end while

    DCHECK(false) << "should not reached here.";
    return false;
}

// returns true if the puyo grounded.
bool FieldRealtime::PlayInternal(Key key, bool* accepted)
{
    bool ground = false;

    KumipuyoPos pos = kumipuyoPos();

    switch (key) {
    case KEY_RIGHT_TURN:
        switch (kumipuyoPos_.r) {
        case 0:
            if (field_.color(kumipuyoPos_.x + 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                *accepted = true;
            } else if (field_.color(kumipuyoPos_.x - 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                kumipuyoPos_.x--;
                *accepted = true;
            } else {
                if (quickturn_ > 0) {
                    kumipuyoPos_.r = 2;
                    kumipuyoPos_.y++;
                    *accepted = true;
                    quickturn_ = 0;
                } else {
                    quickturn_ = FRAMES_QUICKTURN;
                    *accepted = true;
                }
            }
            break;
        case 1:
            if (field_.color(kumipuyoPos_.x, kumipuyoPos_.y - 1) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                *accepted = true;
            } else {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                kumipuyoPos_.y++;
                *accepted = true;
            }
            break;
        case 2:
            if (field_.color(kumipuyoPos_.x - 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                *accepted = true;
            } else if (field_.color(kumipuyoPos_.x + 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
                kumipuyoPos_.x++;
                *accepted = true;
            } else {
                if (quickturn_ > 0) {
                    kumipuyoPos_.r = 0;
                    kumipuyoPos_.y--;
                    *accepted = true;
                    quickturn_ = 0;
                    *accepted = true;
                } else {
                    quickturn_ = FRAMES_QUICKTURN;
                    *accepted = true;
                }
            }
            break;
        case 3:
            kumipuyoPos_.r = (kumipuyoPos_.r + 1) % 4;
            *accepted = true;
            break;
        }
        return false;
    case KEY_LEFT_TURN:
        switch (kumipuyoPos_.r) {
        case 0:
            if (field_.color(kumipuyoPos_.x - 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                *accepted = true;
            } else if (field_.color(kumipuyoPos_.x + 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                kumipuyoPos_.x++;
                *accepted = true;
            } else {
                if (quickturn_ > 0) {
                    kumipuyoPos_.r = 2;
                    kumipuyoPos_.y++;
                    *accepted = true;
                    quickturn_ = 0;
                } else {
                    quickturn_ = FRAMES_QUICKTURN;
                    *accepted = true;
                }
            }
            break;
        case 1:
            kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
            *accepted = true;
            break;
        case 2:
            if (field_.color(kumipuyoPos_.x + 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                *accepted = true;
            } else if (field_.color(kumipuyoPos_.x - 1, kumipuyoPos_.y) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                kumipuyoPos_.x--;
                *accepted = true;
            } else {
                if (quickturn_ > 0) {
                    kumipuyoPos_.r = 0;
                    kumipuyoPos_.y--;
                    *accepted = true;
                    quickturn_ = 0;
                } else {
                    quickturn_ = FRAMES_QUICKTURN;
                    *accepted = true;
                }
            }
            break;
        case 3:
            if (field_.color(kumipuyoPos_.x, kumipuyoPos_.y - 1) == PuyoColor::EMPTY) {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                *accepted = true;
            } else {
                kumipuyoPos_.r = (kumipuyoPos_.r + 3) % 4;
                kumipuyoPos_.y++;
                *accepted = true;
            }
            break;
        }
        return false;
    case KEY_RIGHT:
        if (field_.color(pos.axisX() + 1, pos.axisY()) == PuyoColor::EMPTY &&
            field_.color(pos.childX() + 1, pos.childY()) == PuyoColor::EMPTY) {
            kumipuyoPos_.x++;
            *accepted = true;
        } else {
            *accepted = false;
        }
        break;
    case KEY_LEFT:
        if (field_.color(pos.axisX() - 1, pos.axisY()) == PuyoColor::EMPTY &&
            field_.color(pos.childX() - 1, pos.childY()) == PuyoColor::EMPTY) {
            kumipuyoPos_.x--;
            *accepted = true;
        } else {
            *accepted = false;
        }
        break;
    case KEY_DOWN:
        frames_for_free_fall_ = 0;
        if (field_.color(pos.axisX(), pos.axisY() - 1) == PuyoColor::EMPTY &&
            field_.color(pos.childX(), pos.childY() - 1) == PuyoColor::EMPTY) {
            kumipuyoPos_.y--;
            *accepted = true;
        } else {
            // Ground.
            field_.setPuyoAndHeight(pos.axisX(), pos.axisY(), kumipuyoSeq_.axis(0));
            field_.setPuyoAndHeight(pos.childX(), pos.childY(), kumipuyoSeq_.child(0));
            *accepted = false;
            ground = true;
        }
        break;
    case KEY_UP:
        // When KEY_UP is pressed, nothing happens.
        *accepted = false;
        break;
    case KEY_START:
        *accepted = false;
        break;
    case KEY_NONE:
        *accepted = false;
        break;
    }

    return ground;
}

bool FieldRealtime::Chigiri()
{
    if (chigiri_x_ < 0) {
        KumipuyoPos pos = kumipuyoPos();
        if (field_.color(pos.axisX(), pos.axisY() - 1) == PuyoColor::EMPTY) {
            chigiri_x_ = pos.axisX();
            chigiri_y_ = pos.axisY();
        }
        if (field_.color(pos.childX(), pos.childY() - 1) == PuyoColor::EMPTY) {
            chigiri_x_ = pos.childX();
            chigiri_y_ = pos.childY();
        }
    }

    if (chigiri_x_ < 0) {
        return false;
    }

    int x = chigiri_x_;
    int y = chigiri_y_;
    if (field_.color(x, y - 1) == PuyoColor::EMPTY) {
        field_.setPuyoAndHeight(x, y - 1, field_.color(x, y));
        field_.setPuyoAndHeight(x, y, PuyoColor::EMPTY);
        chigiri_y_--;
        return true;
    } else {
        return false;
    }
}

bool FieldRealtime::Drop1line()
{
    bool ret = false;
    for (int x = 1; x <= CoreField::WIDTH; x++) {
        // Puyo in 14th row will not drop to 13th row. If there is a puyo on
        // 14th row, it'll stay there forever. This behavior is a famous bug in
        // Puyo2.
        for (int y = 1; y < CoreField::MAP_HEIGHT - 3; y++) {
            if (field_.color(x, y) == PuyoColor::EMPTY && field_.color(x, y + 1) != PuyoColor::EMPTY) {
                field_.setPuyoAndHeight(x, y, field_.color(x, y + 1));
                field_.setPuyoAndHeight(x, y + 1, PuyoColor::EMPTY);
                ret = true;
            }
        }
    }
    return ret;
}

void FieldRealtime::PrepareNextPuyo()
{
    kumipuyoPos_ = KumipuyoPos(3, 12, 0);

    if (!kumipuyoSeq_.isEmpty())
        kumipuyoSeq_.dropFront();
    if (FLAGS_delay_wnext)
        delayFramesWNextAppear_ = FRAMES_YOKOKU_DELAY;
    sent_wnext_appeared_ = false;
    simulationState_ = STATE_USER;
}

PlayerFrameData FieldRealtime::playerFrameData() const
{
    return PlayerFrameData(field(), kumipuyoSeq().subsequence(0, 3), kumipuyoPos(), userState(), score(), ojama());
}

Key FieldRealtime::getKey(const Decision& decision) const
{
    if (!decision.isValid())
        return KEY_NONE;

    KumipuyoPos pos = kumipuyoPos();

    LOG(INFO) << "[" << pos.axisX() << ", " << pos.axisY() << "(" << pos.r << ")] -> ["
              << decision.x << "(" << decision.r << ")]";

    vector<KeyTuple> keys;
    KeyTuple next_key;
    if (Ctrl::getControlOnline(field_, KumipuyoPos(decision.x, 1, decision.r), pos, &keys)) {
        LOG(INFO) << Ctrl::buttonsDebugString(keys);

        // Remove redundant key stroke.
        if (pos.r == 3 && keys[0].b1 == KEY_RIGHT_TURN && keys[1].b1 == KEY_LEFT_TURN) {
            next_key = keys[2];
        } else if (pos.r == 1 && keys[0].b1 == KEY_LEFT_TURN &&
                   keys[1].b1 == KEY_RIGHT_TURN) {
            next_key = keys[2];
        } else {
            next_key = keys[0];
        }
    } else {
        LOG(INFO) << "No way...";
        return KEY_NONE;
    }

    if (next_key.b1 != KEY_NONE) {
        return next_key.b1;
    } else if (next_key.b2 != KEY_NONE) {
        return next_key.b2;
    } else {
        return KEY_NONE;
    }
}

int FieldRealtime::reduceOjama(int n)
{
    if (numPendingOjama_ >= n) {
        numPendingOjama_ -= n;
        n = 0;
    } else {
        n -= numPendingOjama_;
        numPendingOjama_ = 0;
    }

    if (numFixedOjama_ >= n) {
        numFixedOjama_ -= n;
        n = 0;
    } else {
        n -= numFixedOjama_;
        numFixedOjama_ = 0;
    }

    return n;
}

// Returns what column should drop how many Ojama puyos. The returned vector
// has 6 elements.
// If there are more than 30 Ojama puyos to drop, all column will have 5.
vector<int> FieldRealtime::determineColumnOjamaAmount()
{
    int dropOjama = numFixedOjama_ >= 30 ? 30 : numFixedOjama_;
    numFixedOjama_ -= dropOjama;

    // Decide which column to drop.
    int positions[6] = {0};
    for (int i = 0; i < 6; i++)
        positions[i] = i;
    for (int i = 1; i < 6; i++)
        swap(positions[i], positions[rand() % (i+1)]);

    vector<int> ret(6, 0);
    int lines = dropOjama / 6;
    dropOjama %= 6;
    for (int i = 0; i < dropOjama; i++) {
        ret[positions[i]] = lines + 1;
    }
    for (int i = dropOjama; i < 6; i++) {
        ret[positions[i]] = lines;
    }

    return ret;
}

Kumipuyo FieldRealtime::kumipuyo(int nth) const
{
    if (nth >= 2 && delayFramesWNextAppear_ > 0)
        return Kumipuyo(PuyoColor::EMPTY, PuyoColor::EMPTY);
    return kumipuyoSeq_.get(nth);
}

PuyoColor FieldRealtime::puyoColor(NextPuyoPosition npp) const
{
    switch (npp) {
    case NextPuyoPosition::CURRENT_AXIS:
        return kumipuyo(0).axis;
    case NextPuyoPosition::CURRENT_CHILD:
        return kumipuyo(0).child;
    case NextPuyoPosition::NEXT1_AXIS:
        return kumipuyo(1).axis;
    case NextPuyoPosition::NEXT1_CHILD:
        return kumipuyo(1).child;
    case NextPuyoPosition::NEXT2_AXIS:
        return kumipuyo(2).axis;
    case NextPuyoPosition::NEXT2_CHILD:
        return kumipuyo(2).child;
    default:
        break;
    }

    DCHECK(false) << static_cast<int>(npp);
    return PuyoColor::EMPTY;
}

void FieldRealtime::skipLevelSelect()
{
    if (simulationState_ == STATE_LEVEL_SELECT) {
        simulationState_ = STATE_USER;
        sleepFor_ = 0;
        PrepareNextPuyo();
        userState_.playable = true;
    }
}
