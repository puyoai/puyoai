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
        int frame = 65536;
        if(enemy.isRensaOngoing()) {
        	frame = enemy.rensaFinishingFrameId() - frameId;
        	fprintf(stderr, "frame: %d\n", frame);
        	if(frame<=0) {
        		frame = 65536;
        	}
        }
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(fast);

        LOG(INFO) << f.toDebugString() << seq.toString();

        int search_turns = 12;
        int time_limit_ms = 600;
        int simCount = 0;
        int counts[32] = {0};
        while(currentTimeInMillis() - start_time_ms < time_limit_ms) {
            ++simCount;
            KumipuyoSeq simSeq = seq;
            if(simSeq.size()<search_turns) {
                simSeq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(search_turns-simSeq.size(), (simCount*1234567891) ^ (frameId*987654321)));
            }
            {
                //simulation
                std::vector<int> bestGenom;
                std::pair<int, int> bestSc(0, -1);
                for(int tryCount=0; tryCount<100; ++tryCount) {
                    std::vector<int> genom(bestGenom.begin(), bestGenom.begin() + myRandInt(bestGenom.size()));
                    CoreField f2 = f;
                    int sc = 0;
                    int maxChain = 0;
                    int ff = 0;
                    for(int i=0; i<(int)genom.size(); ++i) {
                        f2.dropKumipuyo(Decision(genom[i]>>2, genom[i]&3), simSeq.get(i));
                        const auto & re = f2.simulate();
                        sc += re.score;
                        ff += re.frames;
                        maxChain = std::max(maxChain, re.chains);
                    }
                    while((int)genom.size()<simSeq.size() && ff<frame) {
                        int cnt = 0;
                        Decision select;
                        Plan::iterateAvailablePlans(f2, simSeq.subsequence(genom.size(), 1), 1, [&cnt, &select](const RefPlan& plan) {
                            ++cnt;
                            if(myRandInt(cnt)==0) {
                                select = plan.decisions().front();
                            }
                        });
                        f2.dropKumipuyo(select, simSeq.get(genom.size()));
                        const auto & re = f2.simulate();
                        sc += re.score;
                        ff += re.frames;
                        maxChain = std::max(maxChain, re.chains);
                        assert(0<=select.r);
                        assert(select.r<4);
                        genom.push_back((select.x<<2) | (select.r));
                    }
                    std::pair<int, int> sc2(maxChain, sc);
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
        fprintf(stderr, "simCount: %d\n", simCount);
        int bestAns = -1;
        int bestCnt = 0;
        for(int i=0; i<32; ++i) {
        	if(1<=counts[i]) {
        		//fprintf(stderr, "%d => %d\n", i, counts[i]);
        	}
        	if(bestCnt<counts[i]) {
        		bestCnt = counts[i];
        		bestAns = i;
        	}
        }
        assert(bestAns!=-1);
        return DropDecision(Decision(bestAns>>2, bestAns&3));
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
