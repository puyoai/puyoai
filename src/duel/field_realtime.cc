#include "duel/field_realtime.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "duel/frame_context.h"

using namespace std;

DEFINE_bool(delay_wnext, true, "Delay wnext appear");

// (n) means frames to transit to the next state.
// TODO(mayah): quick?
//
// STATE_LEVEL_SELECT
//  v (6)
// STATE_PREPARING_NEXT <--------+
//  v (0)                        |
// STATE_PLAYABLE (?)            |
//  v (0)                        |
// STATE_DROPPING (?) <--+       |
//  v (10 or 0 if quick) |       |
// STATE_GROUNDING       |       |
//  v (0)                |       |
// STATE_VANISH          |       |
//  v  v (25)            | (0)   |
//  v  STATE_VANISHING --+       |
//  v (0)                        |
// STATE_OJAMA_DROPPING          |
//  v (10)                       | (6)
// STATE_OJAMA_GROUNDING --------+
//  v
// STATE_DEAD

FieldRealtime::FieldRealtime(int playerId, const KumipuyoSeq& seq) :
    playerId_(playerId)
{
    // Since we don't use the first kumipuyo, we need to put EMPTY/EMPTY.
    vector<Kumipuyo> kps;
    kps.push_back(Kumipuyo(PuyoColor::EMPTY, PuyoColor::EMPTY));
    kps.insert(kps.end(), seq.underlyingData().begin(), seq.underlyingData().end());
    kumipuyoSeq_ = KumipuyoSeq(kps);

    init();
}

void FieldRealtime::init()
{
    userState_.playable = false;
    sleepFor_ = 30;

    ojama_position_ = vector<int>(6, 0);
    ojama_dropping_ = false;
    current_chains_ = 1;
    delayFramesWNextAppear_ = 0;
    sent_wnext_appeared_ = false;
    drop_animation_ = false;
}

bool FieldRealtime::onStateLevelSelect()
{
    transitToStatePreparingNext();
    // We always send 'grounded' before the initial puyo appear.
    userState_.grounded = true;
    return false;
}

void FieldRealtime::transitToStatePreparingNext()
{
    simulationState_ = SimulationState::STATE_PREPARING_NEXT;
    sleepFor_ = FRAMES_PREPARING_NEXT;
    userState_.playable = false;

    mks_ = MovingKumipuyoState(KumipuyoPos(3, 12, 0));

    if (!kumipuyoSeq_.isEmpty())
        kumipuyoSeq_.dropFront();
    if (FLAGS_delay_wnext)
        delayFramesWNextAppear_ = FRAMES_NEXT2_DELAY + FRAMES_PREPARING_NEXT;
    sent_wnext_appeared_ = false;
    allowsQuick_ = false;
}

bool FieldRealtime::onStatePreparingNext()
{
    simulationState_ = SimulationState::STATE_PLAYABLE;
    userState_.playable = true;
    userState_.decisionRequest = true;
    return false;
}

bool FieldRealtime::onStatePlayable(const KeySet& keySet, bool* accepted)
{
    userState_.playable = true;
    current_chains_ = 1;
    // TODO(mayah): We're always accepting KeySet? Then do we need to take |accepted| here?
    *accepted = true;

    bool downAccepted = false;
    PuyoController::moveKumipuyo(field_, keySet, &mks_, &downAccepted);
    if (downAccepted)
        ++score_;

    if (mks_.grounded) {
        field_.setPuyoAndHeight(mks_.pos.axisX(), mks_.pos.axisY(), kumipuyoSeq_.axis(0));
        field_.setPuyoAndHeight(mks_.pos.childX(), mks_.pos.childY(), kumipuyoSeq_.child(0));
        userState_.playable = false;
        userState_.grounded = true;
        dropVelocity_ = INITIAL_DROP_VELOCITY;
        dropAmount_ = 0.0;
        drop_animation_ = false;
        sleepFor_ = 0;
        simulationState_ = SimulationState::STATE_DROPPING;
    }

    return true;
}

bool FieldRealtime::onStateDropping()
{
    if (drop1Frame()) {
        drop_animation_ = true;
        return true;
    }

    bool wasDropping = drop_animation_;
    drop_animation_ = false;
    if (!wasDropping && allowsQuick_)
        sleepFor_ = 0;
    else
        sleepFor_ = FRAMES_GROUNDING;

    simulationState_ = SimulationState::STATE_GROUNDING;
    return false;
}

bool FieldRealtime::onStateGrounding()
{
    simulationState_ = SimulationState::STATE_VANISH;
    return false;
}

bool FieldRealtime::onStateVanish(FrameContext* context)
{
    // TODO(mayah): field_ looks inconsistent in some reason.
    // Let's recalculate the height.
    for (int x = 1; x <= CoreField::WIDTH; ++x)
        field_.recalcHeightOn(x);

    int score = field_.vanishOnly(current_chains_++);
    if (score == 0) {
        if (context)
            context->commitOjama();
        sleepFor_ = 0;
        drop_animation_ = false;
        simulationState_ = SimulationState::STATE_OJAMA_DROPPING;
        userState_.chainFinished = true;
        return false;
    }

    // Here, something has been vanished.
    score_ += score;

    // Set Yokoku Ojama.
    if (score_ - scoreConsumed_ >= SCORE_FOR_OJAMA) {
        int attack_ojama = (score_ - scoreConsumed_) / SCORE_FOR_OJAMA;
        if (context)
            context->sendOjama(attack_ojama);
        scoreConsumed_ = score_ / SCORE_FOR_OJAMA * SCORE_FOR_OJAMA;
    }

    // After ojama is calculated, we add ZENKESHI score,
    // because score for ZENKESHI is added, but not used for ojama calculation.
    hasZenkeshi_ = false;
    if (field_.isZenkeshiPrecise()) {
        score_ += ZENKESHI_BONUS;
        hasZenkeshi_ = true;
    }

    simulationState_ = SimulationState::STATE_VANISHING;
    dropVelocity_ = MAX_DROP_VELOCITY;
    dropAmount_ = 0.0;
    sleepFor_ = FRAMES_VANISH_ANIMATION;
    allowsQuick_ = true;
    return false;
}

bool FieldRealtime::onStateVanishing()
{
    simulationState_ = SimulationState::STATE_DROPPING;
    return false;
}

bool FieldRealtime::onStateOjamaDropping()
{
    if (!ojama_dropping_) {
        ojama_position_ = determineColumnOjamaAmount();
        for (int i = 0; i < 6; i++) {
            if (ojama_position_[i] > 0) {
                ojama_dropping_ = true;
                break;
            }
        }

        dropVelocity_ = INITIAL_DROP_VELOCITY;
        dropAmount_ = 0.0;
    }

    bool ojamaWasDropping = ojama_dropping_;
    if (ojama_dropping_) {
        for (int i = 0; i < 6; i++) {
            if (ojama_position_[i] > 0) {
                if (field_.color(i + 1, 13) == PuyoColor::EMPTY) {
                    field_.setPuyoAndHeight(i + 1, 13, PuyoColor::OJAMA);
                    ojama_position_[i]--;
                }
            }
        }

        if (drop1Frame())
            return true;
    }

    if (!ojamaWasDropping) {
        sleepFor_ = 0;
    } else {
        // TODO(mayah): We need to sleep more. 1 ojama -> +4 frames, 30 ojama -> 16frames, etc.
        sleepFor_ = FRAMES_GROUNDING;
    }

    simulationState_ = SimulationState::STATE_OJAMA_GROUNDING;
    return false;
}

bool FieldRealtime::onStateOjamaGrounding()
{
    if (ojama_dropping_)
        userState_.ojamaDropped = true;
    ojama_dropping_ = false;

    if (field_.color(3, 12) != PuyoColor::EMPTY) {
        simulationState_ = SimulationState::STATE_DEAD;
        return false;
    }

    transitToStatePreparingNext();
    return false;
}

bool FieldRealtime::onStateDead()
{
    return true;
}

// Returns true if a key input is accepted.
bool FieldRealtime::playOneFrame(const KeySet& keySet, FrameContext* context)
{
    userState_.clear();

    if (delayFramesWNextAppear_ > 0)
        --delayFramesWNextAppear_;

    if (delayFramesWNextAppear_ == 0 && !sent_wnext_appeared_) {
        userState_.wnextAppeared = true;
        sent_wnext_appeared_ = true;
    }

    // Loop until some functionality consumes this frame.
    while (true) {
        if (sleepFor_ > 0) {
            sleepFor_--;
            return false;
        }

        switch (simulationState_) {
        case SimulationState::STATE_LEVEL_SELECT:
            if (onStateLevelSelect())
                return false;
            continue;
        case SimulationState::STATE_PREPARING_NEXT:
            if (onStatePreparingNext())
                return false;
            continue;
        case SimulationState::STATE_PLAYABLE: {
            bool accepted = false;
            if (onStatePlayable(keySet, &accepted))
                return accepted;
            continue;
        }
        case SimulationState::STATE_DROPPING:
            if (onStateDropping())
                return false;
            continue;
        case SimulationState::STATE_GROUNDING:
            if (onStateGrounding())
                return false;
            continue;
        case SimulationState::STATE_VANISH:
            if (onStateVanish(context))
                return false;
            continue;
        case SimulationState::STATE_VANISHING:
            if (onStateVanishing())
                return false;
            continue;
        case SimulationState::STATE_OJAMA_DROPPING:
            if (onStateOjamaDropping())
                return false;
            continue;
        case SimulationState::STATE_OJAMA_GROUNDING:
            if (onStateOjamaGrounding())
                return false;
            continue;
        case SimulationState::STATE_DEAD:
            if (onStateDead())
                return false;
            continue;
        }

        CHECK(false) << "Unknown state?";
    }  // end while

    DCHECK(false) << "should not reached here.";
    return false;
}

bool FieldRealtime::drop1Frame()
{
    double velocity = dropVelocity_;
    dropVelocity_ = min(dropVelocity_ + DROP_ACCELARATION_PER_FRAME, MAX_DROP_VELOCITY);

    bool needToDrop = false;
    dropAmount_ += velocity;
    if (dropAmount_ >= DROP_1BLOCK_THRESHOLD) {
        dropAmount_ -= DROP_1BLOCK_THRESHOLD;
        needToDrop = true;
    }

    bool stillDropping = false;
    // Puyo in 14th row will not drop to 13th row. If there is a puyo on
    // 14th row, it'll stay there forever. This behavior is a famous bug in
    // Puyo2.
    for (int x = 1; x <= CoreField::WIDTH; x++) {
        for (int y = 1; y < 13; y++) {
            if (field_.color(x, y) != PuyoColor::EMPTY)
                continue;

            if (field_.color(x, y + 1) != PuyoColor::EMPTY) {
                stillDropping = true;
                if (needToDrop) {
                    field_.setPuyoAndHeight(x, y, field_.color(x, y + 1));
                    field_.setPuyoAndHeight(x, y + 1, PuyoColor::EMPTY);
                }
            }
        }
    }

    return stillDropping;
}

PlayerFrameData FieldRealtime::playerFrameData() const
{
    return PlayerFrameData(field(), kumipuyoSeq().subsequence(0, 3), kumipuyoPos(), userState(), score(), ojama());
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
    CHECK(simulationState_ == SimulationState::STATE_LEVEL_SELECT);
    transitToStatePreparingNext();
}

void FieldRealtime::skipPreparingNext()
{
    CHECK(simulationState_ == SimulationState::STATE_PREPARING_NEXT);
    simulationState_ = SimulationState::STATE_PLAYABLE;
    sleepFor_ = 0;
}
