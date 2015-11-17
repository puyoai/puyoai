#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cassert>
#include <random>

#include "base/base.h"
#include "core/plan/plan.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "base/time.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/puyo_controller.h"

unsigned int myRandInt(unsigned int v) {
    static std::mt19937 rnd(1);
    return ((unsigned long long)rnd() * v) >> 32;
}

class ColunAI : public AI {
public:
    ColunAI(int argc, char* argv[]) : AI(argc, argv, "colun") {}
    ~ColunAI() override {}

    DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override
    {
        long long start_time_ms = currentTimeInMillis();
        static const int maxFrame = 300;
        int frame = maxFrame;
        if(enemy.isRensaOngoing()) {
            frame = enemy.rensaFinishingFrameId() - frameId;
            //fprintf(stderr, "frame: %d\n", frame);
            if(frame<=0) {
                frame = maxFrame;
            }
        }
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(fast);

        LOG(INFO) << f.toDebugString() << seq.toString();

        static const Decision DECISIONS[] = {
            Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1),
            Decision(5, 1), Decision(1, 2), Decision(2, 2), Decision(3, 2),
            Decision(4, 2), Decision(5, 2), Decision(6, 2), Decision(1, 1),
            Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3),
            Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0),
            Decision(5, 0), Decision(6, 0),
        };

        int search_turns = 20;
        int time_limit_ms = 600;
        int simCount = 0;
        int counts[22] = {0};
        while(currentTimeInMillis() - start_time_ms < time_limit_ms) {
            ++simCount;
            KumipuyoSeq simSeq = seq;
            if(simSeq.size()<search_turns) {
                simSeq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(search_turns-simSeq.size(), (simCount*1234567891) ^ (frameId*987654321)));
            }
            {
                //simulation
                std::vector<int> bestGenom;
                std::pair<int, std::pair<int, int> > bestSc(-100, std::pair<int, int>(0, 0));
                for(int tryCount=0; tryCount<200; ++tryCount) {
                    std::vector<int> genom;//(bestGenom.begin(), bestGenom.begin() + myRandInt(bestGenom.size()));
                    CoreField f2 = f;
                    int sc = 0;
                    int maxChain = 0;
                    int mSc = 0;
                    int ff = 0;
                    bool dead = false;
                    for(int i=0; i<(int)genom.size(); ++i) {
                        auto & de = DECISIONS[genom[i]];
                        int dropFrames = f2.framesToDropNext(de);
                        f2.dropKumipuyo(de, simSeq.get(i));
                        const auto & re = f2.simulate();
                        if(!f2.isEmpty(3, 12)) {
                            dead = true;
                            break;
                        }
                        bool zenkeshi = (f2.isZenkeshi() && i<seq.size());
                        sc += re.score;
                        ff += dropFrames + re.frames;
                        maxChain = std::max(maxChain, re.chains + (zenkeshi ? 3 : 0));
                        mSc = std::max(mSc, re.score);
                    }
                    if(dead) {
                        continue;
                    }
                    while((int)genom.size()<simSeq.size() && ff<frame) {
                        std::vector<int> candidates;
                        auto & puyo = simSeq.get(genom.size());
                        if(puyo.axis==puyo.child) {
                            for(int v=0; v<11; ++v) {
                                candidates.push_back(v);
                            }
                        }
                        else {
                            for(int v=0; v<22; ++v) {
                                candidates.push_back(v);
                            }
                        }
                        while(true) {
                            if(candidates.empty()) {
                                dead = true;
                                break;
                            }
                            int i = myRandInt(candidates.size());
                            int v = candidates[i];
                            candidates[i] = candidates.back();
                            candidates.pop_back();
                            auto & de = DECISIONS[v];
                            if(!PuyoController::isReachable(f2, de)) {
                                continue;
                            }
                            int dropFrames = f2.framesToDropNext(de);
                            if(!f2.dropKumipuyo(de, simSeq.get(genom.size()))) {
                                dead = true;
                                break;
                            }
                            const auto & re = f2.simulate();
                            if(!f2.isEmpty(3, 12)) {
                                dead = true;
                                break;
                            }
                            sc += re.score;
                            ff += dropFrames + re.frames;
                            bool zenkeshi = (f2.isZenkeshi() && (int)genom.size()<seq.size());
                            maxChain = std::max(maxChain, re.chains + (zenkeshi ? 3 : 0));
                            mSc = std::max(mSc, re.score);
                            genom.push_back(v);
                            break;
                        }
                        if(dead) {
                            break;
                        }
                    }
                    if(dead) {
                        continue;
                    }
                    std::pair<int, std::pair<int, int> > sc2(maxChain<3 ? -10+maxChain : maxChain, std::pair<int, int>(mSc, -sc));
                    if(bestSc<sc2) {
                        bestSc = sc2;
                        bestGenom = genom;
                    }
                }
                if(!bestGenom.empty()) {
                    ++counts[bestGenom.front()];
                }
            }
        }
        //fprintf(stderr, "simCount: %d\n", simCount);
        int bestAns = 0;
        int bestCnt = 0;
        for(int i=0; i<22; ++i) {
            if(1<=counts[i]) {
                //fprintf(stderr, "%d => %d\n", i, counts[i]);
            }
            if(bestCnt<counts[i]) {
                bestCnt = counts[i];
                bestAns = i;
            }
        }
        return DropDecision(DECISIONS[bestAns]);
    }
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    ColunAI(argc, argv).runLoop();
    return 0;
}
