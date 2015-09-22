#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "base/executor.h"
#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/score.h"
#include "solver/endless.h"
#include "solver/puyop.h"

#include <cstdio>

#include <unordered_set>
#include <unordered_map>
#include <future>

#include <iostream>

using namespace std;

struct State
{
    CoreField field;

    double score;
    int frames;
    int fired_chains;

    int pending_enemy_score;
    int dropped_frames;

    Decision decision;
    Decision first_decision;
    int prev_q_index;

    bool fired_main_chain() const
    {
        return fired_chains > 0;
    }

    bool has_pending_ojama() const
    {
        return pending_enemy_score > 0;
    }
};

void next_states(const State& current_state, const Kumipuyo& kumipuyo, const int good_chains, unordered_set<int64_t>& visited, int qi, vector<State>& state_q, vector<State>& fired)
{
    auto drop_callback = [&](const RefPlan& plan)
    {
        DCHECK(plan.decisions().size() == 1);

        CoreField field = plan.field();
        RensaResult rensa_result = plan.rensaResult();

        const int64_t field_hash = plan.field().hash();
        if (visited.count(field_hash))
            return;
        visited.insert(field_hash);

        int frames = current_state.frames + plan.totalFrames() + rensa_result.frames;
        int pending_enemy_score = current_state.pending_enemy_score;

        if (rensa_result.chains > 0)
        {
            State next = State {
                .field = field,
                .score = (double)rensa_result.score,
                .frames = frames,
                .fired_chains = rensa_result.chains,
                .pending_enemy_score = current_state.pending_enemy_score,
                .dropped_frames = current_state.dropped_frames,
                .decision = plan.decision(0),
                .first_decision = current_state.prev_q_index == -1 ? plan.decision(0) : current_state.first_decision,
                .prev_q_index = qi
            };
            fired.push_back(next);

            if (rensa_result.chains >= good_chains)
                return;
        }

        if (current_state.has_pending_ojama() && frames >= current_state.dropped_frames)
        {
            frames += field.fallOjama(pending_enemy_score / SCORE_FOR_OJAMA / 6);
            pending_enemy_score = 0;
        }
        if (!field.isEmpty(3, 12))
            return;

        int max_chains = 0;
        int highest_ignition_y = 0;
        bool prohibits[FieldConstant::MAP_WIDTH]{};
        auto complement_callback = [&max_chains, &highest_ignition_y](CoreField&& complemented_field, const ColumnPuyoList& puyo_list)
        {
            int ignition_y = -1;
            for (int x = 1; x <= 6; ++x)
                if (puyo_list.sizeOn(x) > 0)
                    ignition_y = complemented_field.height(x);
            DCHECK(ignition_y != -1);

            RensaResult rensa_result = complemented_field.simulate();
            if (make_tuple(rensa_result.chains, ignition_y) > make_tuple(max_chains, highest_ignition_y))
            {
                max_chains = rensa_result.chains;
                highest_ignition_y = ignition_y;
            }
        };
        RensaDetector::detectByDropStrategy(field, prohibits, PurposeForFindingRensa::FOR_FIRE, 2, 13, complement_callback);


        double score = 0;

        if (max_chains >= 2)
            score += max_chains * 1000;

        double ave_height = 0;
        for (int x = 1; x <= 6; ++x)
            ave_height += field.height(x);
        ave_height /= 6;

        double u_score = 0;
        for (int x = 1; x <= 6; ++x)
        {
            static const int good_diff[] = { 0, 2, 0, -2, -2, 0, 2, 0 };
            u_score -= abs((field.height(x) - ave_height) - good_diff[x]);
        }
        score += 60 * u_score;

        score += 10 * (highest_ignition_y - ave_height);

        for (int x = 1; x <= 6; ++x)
        {
            static const double coef[] = { 0, 1, 3, 0, 3, 2, 1, 0 };
            score -= coef[x] * !field.isEmpty(x, 13);
        }


        State next = State {
            .field = field,
            .score = score,
            .frames = frames,
            .fired_chains = 0,
            .pending_enemy_score = pending_enemy_score,
            .dropped_frames = current_state.dropped_frames,
            .decision = plan.decision(0),
            .first_decision = current_state.prev_q_index == -1 ? plan.decision(0) : current_state.first_decision,
            .prev_q_index = qi
        };
        state_q.push_back(next);
    };
    Plan::iterateAvailablePlans(current_state.field, {kumipuyo}, 1, drop_callback);
}

struct BeamSearchResult
{
    vector<Decision> decisions;
    int chains;
};
BeamSearchResult beamsearch(const CoreField& start_field, const KumipuyoSeq& seq, const int frame_id, const PlayerState& me, const PlayerState& enemy, const int turns, const int good_chains)
{
    UNUSED_VARIABLE(me);

    const int BEAM_WIDTH = 400;
    CHECK(turns >= 0);
    CHECK(seq.size() >= turns);

    vector<vector<State>> fired(turns + 1);
    vector<vector<State>> state_q(turns + 1);
    const auto make_decisions = [&](const State& state, int turn)
    {
        vector<Decision> decisions;
        decisions.push_back(state.decision);
        for (int i = turn - 1, qi = state.prev_q_index; i > 0; --i)
        {
            CHECK(0 <= qi && qi < (int)state_q[i].size());
            decisions.push_back(state_q[i][qi].decision);
            qi = state_q[i][qi].prev_q_index;
        }
        reverse(decisions.begin(), decisions.end());
        CHECK(!decisions.empty());
        return decisions;
    };

    const State init_state = State {
        .field = start_field,
        .score = 0,
        .frames= 0,
        .fired_chains = 0,
        .pending_enemy_score = enemy.isRensaOngoing() ? scoreForOjama(me.totalOjama(enemy)) : 0,
        .dropped_frames = enemy.isRensaOngoing() ? enemy.rensaFinishingFrameId() - frame_id : 100000000,
        .decision = Decision(0, 0),
        .first_decision = Decision(0, 0),
        .prev_q_index = -1
    };
    state_q[0].push_back(init_state);

    Decision first_decision_for_max_chains;
    int max_chains = 0;
    for (int turn = 0; turn < turns; ++turn)
    {
        unordered_set<int64_t> visited;
        state_q[turn + 1].reserve(state_q[turn].size() * 11);
        fired[turn + 1].reserve(state_q[turn].size());

        for (int qi = 0; qi < (int)state_q[turn].size(); ++qi)
        {
            next_states(state_q[turn][qi], seq.get(turn), good_chains, visited, qi, state_q[turn + 1], fired[turn + 1]);
        }

        for (auto& state : fired[turn + 1])
        {
            if (state.fired_chains > max_chains)
            {
                max_chains = state.fired_chains;
                first_decision_for_max_chains = state.first_decision;
            }
        }


        if (state_q[turn + 1].empty())
            break;

        std::sort(state_q[turn + 1].begin(), state_q[turn + 1].end(), [](const State& a, const State& b){ return a.score > b.score; });
        if (state_q[turn + 1].size() > BEAM_WIDTH)
            state_q[turn + 1].erase(state_q[turn + 1].begin() + BEAM_WIDTH, state_q[turn + 1].end());

        bool skip_search = max_chains >= good_chains && state_q[turn + 1][0].first_decision == first_decision_for_max_chains && all_of(state_q[turn + 1].begin(), state_q[turn + 1].end()
                ,[&](const State& s){ return s.first_decision == state_q[turn + 1][0].first_decision; });
        if (skip_search)
            break;
    }

    for (int turn = 0; turn <= turns; ++turn)
    {
        std::sort(fired[turn].begin(), fired[turn].end(), [](const State& a, const State& b){ return a.score > b.score; });
        for (const State& state : fired[turn])
        {
            if (state.fired_chains == max_chains)
            {
                return BeamSearchResult {
                    .decisions = make_decisions(state, turn),
                    .chains = max_chains
                };
            }
        }
    }

    // no chains. select survive decision
//     for (int turn = turns; turn > 0; --turn)
//     {
//         for (const State& state : state_q[turn])
//         {
//             return BeamSearchResult {
//                 .decisions = make_decisions(state, turn),
//                 .chains = 0
//             };
//         }
//     }

    return BeamSearchResult {
        .decisions = {},
        .chains = -1
    };
}

int count_color_puyos_connected_from_start(const CoreField& field)
{
    bool visited[14][8]{};
    std::function<void (int, int)> dfs = [&](int x, int y) {
        if (y > 13 || visited[y][x] || (!field.isEmpty(x, y) && !isNormalColor(field.color(x, y))))
            return;
        visited[y][x] = true;

        static const int dx[] = { 0, 1, 0, -1 };
        static const int dy[] = { 1, 0, -1, 0 };
        for (int dir = 0; dir < 4; ++dir)
            dfs(x + dx[dir], y + dy[dir]);
    };
    dfs(3, 13);

    int c = 0;
    for (int y = 1; y <= 13; ++y)
        for (int x = 1; x <= 6; ++x)
            if (visited[y][x] && field.isNormalColor(x, y))
                ++c;
    return c;
}

DEFINE_int32(seen, 1000, "max number of seen seq");
DEFINE_int32(max_turns, 100000000, "max turn");
DEFINE_int32(ss, 1, "sets the random seed. When negative, seed will be chosen at random.");
DEFINE_int32(loop, 0, "run loops");

class TakaptAI : public AI
{
public:
    TakaptAI(int argc, char* argv[]) : AI(argc, argv, "takapt") {}
    virtual ~TakaptAI() {}

    virtual DropDecision think(int frameId, const CoreField& f, const KumipuyoSeq& seq,
                               const PlayerState& me, const PlayerState& enemy, bool fast) const override
    {
        UNUSED_VARIABLE(frameId);
        UNUSED_VARIABLE(me);
        UNUSED_VARIABLE(enemy);
        UNUSED_VARIABLE(fast);
        return eval(f, seq.subsequence(0, min(seq.size(), FLAGS_seen)), frameId, me, enemy, fast);
    }

    void onGameWillBegin(const FrameRequest&) override
    {
        current_turn = 0;
    }

    void onDecisionRequestedForMe(const FrameRequest&) override
    {
        ++current_turn;
    }

private:
    DropDecision eval(const CoreField& f, const KumipuyoSeq& nexts, int frame_id, const PlayerState& me, const PlayerState& enemy, bool fast) const
    {
        const auto start_time = currentTimeInMillis();

        LOG(INFO) << f.toDebugString() << nexts.toString();


        cerr << "turn: " << current_turn << endl;
        cerr << "seq size: " << nexts.size() << endl;
        int TL = 600;
        int RETRY_TL = 300;
        int SIMULATIONS = 5;
        if (FLAGS_loop == 0)
        {
//             if (nexts.size() <= 4)
//             {
//                 SIMULATIONS = 10;
//                 TL = 1000;
//             }
        }
        else
        {
            TL = 1000000;
            RETRY_TL = 0;
        }
        if (fast)
        {
            TL = 30;
            RETRY_TL = 15;
            SIMULATIONS = 1;
            cerr << "fast" + string(20, '!') << endl;
        }

        int missed_search = 0;
        int good_chains = min(14, count_color_puyos_connected_from_start(f) / 5 + 4);
        vector<int> chains[7][4];
        int simu_i;
        for (simu_i = 0; ((simu_i < SIMULATIONS && good_chains > 0) || currentTimeInMillis() - start_time < RETRY_TL) && currentTimeInMillis() - start_time < TL; )
        {
            const int search_turns = min(FLAGS_max_turns - current_turn,
                    max(max(10, nexts.size()), min(50, ((6 * 13) - f.countPuyos()) / 2 + 4)));

            KumipuyoSeq seq = nexts;
            seq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(search_turns, frame_id + simu_i));

            BeamSearchResult result = beamsearch(f, seq, frame_id, me, enemy, search_turns, good_chains);
            ++simu_i;

            if (result.chains == -1)
            {
                ++missed_search;
                good_chains = max(good_chains - 1, 1);
            }
            else
            {
                CHECK(!result.decisions.empty());
                Decision first_decision = result.decisions.front();
                CHECK(first_decision.isValid());
                chains[first_decision.axisX()][first_decision.rot()].push_back(result.chains);
                if ((int)chains[first_decision.axisX()][first_decision.rot()].size() >= (SIMULATIONS + 1) / 2 && currentTimeInMillis() - start_time > RETRY_TL)
                {
                    break;
                }
            }
        }
        cerr << "simulations: " << simu_i << endl;
        cerr << "success search: " << simu_i - missed_search << endl;
        cerr << "missed search: " << missed_search << endl;

        double exp_chains = 0;
        double drop_score = -1;
        Decision best_decision(3, 0);
        for (int x = 1; x <= 6; ++x)
        {
            for (int rot = 0; rot < 4; ++rot)
            {
                if (!chains[x][rot].empty())
                {
                    int sum_chains = accumulate(chains[x][rot].begin(), chains[x][rot].end(), 0);
                    double score = (double)sum_chains;
                    fprintf(stderr, "%d, %d: %d\n", x, rot, (int)chains[x][rot].size());
                    if (score > drop_score)
                    {
                        exp_chains = (double)accumulate(chains[x][rot].begin(), chains[x][rot].end(), 0) / chains[x][rot].size();
                        drop_score = score;
                        best_decision = Decision(x, rot);
                    }
                }
            }
        }
        cerr << exp_chains << endl;
        cerr << "time: " << currentTimeInMillis() - start_time << "ms" << endl;
        cerr << endl;

        return DropDecision(best_decision);
    }

    int current_turn = -1;
};


struct RunResult
{
    EndlessResult result;
    string message;
};
RunResult run(int seed)
{
    TakaptAI* ai = new TakaptAI({}, 0);

    Endless endless(std::move(std::unique_ptr<AI>(ai)));
//     endless.setVerbose(FLAGS_show_field);

    KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(seed);
    EndlessResult result = endless.run(seq);

    stringstream ss;
    ss << "seed: " << seed << endl;
    ss << seq.toString() << endl;
    ss << makePuyopURL(seq, result.decisions) << endl;
    ss << "score = " << result.score << " rensa = " << result.maxRensa;
    ss << endl;

    return {result, ss.str()};
}

void run_loop(int num, const int start_seed, bool print_info)
{
    unique_ptr<Executor> executor = Executor::makeDefaultExecutor();
    vector<promise<RunResult>> promise_results(num);
    for (int i = 0; i < num; ++i)
    {
        executor->submit(std::move([i, start_seed, &promise_results]() {
                promise_results[i].set_value(run(start_seed + i));
                cout << "done: " << i << endl;
        }));
    }

    vector<RunResult> results;
    for (auto& pr : promise_results)
        results.push_back(pr.get_future().get());

    int count_score[30 * 10000]{};
    int count_chains_[30]{};
    int* count_chains = count_chains_ + 1;
    for (int i = 0; i < num; ++i)
    {
        RunResult result = results[i];
        ++count_chains[result.result.maxRensa];
        ++count_score[result.result.score / 10000];

        cout << result.message << endl;
    }

    if (print_info)
    {
        int valid_num = 0;
        double ave_chains = 0;
        for (int i = -1; i <= 20; ++i)
        {
            printf("%2d: %4d\n", i, count_chains[i]);
            if (i >= 0)
            {
                ave_chains += count_chains[i] * i;
                valid_num += count_chains[i];
            }
        }
        ave_chains /= valid_num;
        cout << "ave chains: " << ave_chains << endl;

        for (int i = 3; i <= 15; ++i)
        {
            printf("%6d: %4d\n", i * 10000, count_score[i]);
        }
        double ave_score = 0;
        for (auto& result : results)
            ave_score += result.result.score;
        ave_score /= valid_num;
        cout << "ave score: " << ave_score << endl;
    }
}


int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (FLAGS_loop == 0)
        TakaptAI(argc, argv).runLoop();
    else
        run_loop(FLAGS_loop, FLAGS_ss, FLAGS_loop > 1);
}
