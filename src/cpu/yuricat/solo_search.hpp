/*
 solo_search.hpp
 yuricat(Katsuki Ohto)
 */

// takaptさん方式のひとり探索

#ifndef PUYOAI_YURICAT_SOLOSEARCH_HPP_
#define PUYOAI_YURICAT_SOLOSEARCH_HPP_

#include "puyo.hpp"
#include "evaluation.hpp"

namespace Yuricat{
    
    constexpr int N_MAX_SOLO_SEARCH_TURNS = 50;
    
    // 共有情報
    struct SoloSearchResult{
        KumipuyoSeq seq;
        int turn, chain, score;
        Decision first;
        //float wp[24];
    };
    
    struct SoloSearchResults{
        mutable std::mutex mutex_;
        vector<SoloSearchResult> result;
        
        void updateSequence(const int turn, const Kumipuyo& tsumo){
            // 新しく確定したので、合致しない結果を外す
            mutex_.lock();
            int size = result.size();
            for(int i = 0; i < size; ++i){
                if(result[i].seq.get(turn) != tsumo){ // 不一致
                    if(i + 1 < size){
                        result[i] = result[size - 1];
                    }
                    --size;
                }
            }
            mutex_.unlock();
        }
        DropDecision bestFirst()const{
            //map<Decision, tuple<int, int>> m;
            
            map<Decision, tuple<double, double>> m;
            // tupleは(回数, 評価和)
            
            mutex_.lock();
            if(result.size() == 0){ // 結果が無かった
                return DropDecision(Decision(), "No Result...");
            }
            int miss = 0;
            for(const auto& r : result){
                if(r.chain > 0){
                    get<0>(m[r.first]) += 1;
                    //get<1>(m[r.first]) += r.chain; // 自動的に0に初期化されるのでOK
                    //get<1>(m[r.first]) += kMinChainScore[r.chain];
                    get<1>(m[r.first]) += r.score;
                    //get<1>(m[r.first]) += scoreToWP(kMinChainScore[r.chain]);
                    //cerr << r.chain << endl;
                }else if(r.chain == 0){
                    miss += 1;
                } 
            }
            Decision best;
            tuple<int, double> best_chain = make_tuple(0, 0);
            
            for(const auto& ds : m){
                if(get<1>(ds.second) > get<1>(best_chain)){
                    best = ds.first;
                    best_chain = ds.second;
                }
            }
            mutex_.unlock();
            cerr << "result size = " << result.size() << endl;
            
            double avg_chain = (get<0>(best_chain) + miss == 0) ? 0 : (get<1>(best_chain) / float(get<0>(best_chain) + miss));
            //cerr << "avg chain = " << avg_chain << endl;
            cerr << "avg score = " << avg_chain << endl;
            std::ostringstream oss;
            oss << "Avg Score = " << avg_chain << " ( " << get<0>(best_chain) << " / "<< result.size() << " ) " << "Death Rate = " << (miss / (double)result.size());
            return DropDecision(best, oss.str());
        }
        void proceed(const Decision& decision){
            // ターンを進める
            UNUSED_VARIABLE(decision);
            mutex_.lock();
            result.clear();
            mutex_.unlock();
        }
        void pushResult(const SoloSearchResult& r){
            mutex_.lock();
            result.push_back(r);
            mutex_.unlock();
        }
        void init(){
            mutex_.lock();
            result.clear();
            mutex_.unlock();
        }
    };
    
    SoloSearchResults g_soloSearchResults[2];
    
    struct SoloNode{
        CoreField field;
        int score;
        int frame;
        int chain;
        int pending_score;
        int pending_frame;
        Decision decision, first_decision;
        int previous_index;
    };
    
    struct FiredResult{
        int chain, score;
        Decision decision, first_decision;
        int previous_index;
    };
    
    template<class node_t, class fired_result_t>
    void genNextSoloStates(const int player,
                           const node_t& node,
                           const int node_index,
                           const Kumipuyo& dropped,
                           const int ojama_seed,
                           const int chain_border,
                           unordered_set<int64_t>& visited,
                           vector<node_t>& next_states,
                           vector<fired_result_t>& fired_results){
        //bool mate = false;
        
        // 次の状態を生成し、オープンキューに追加
        auto drop_callback = [&](const RefPlan& plan){
            DCHECK(plan.decisions().size() == 1);
            
            //if(mate){ return; } // すでに勝ちがあれば探索終了
            
            RensaResult rensa_result = plan.rensaResult(); // 連鎖結果
            CoreField field = plan.field();
            
            const int64_t after_half_side_hash = field.hash(); // 連鎖後の片側の盤面ハッシュ値
            //cerr << after_half_side_hash << ", ";
            if(visited.find(after_half_side_hash) != visited.end()){ /*CERR << "goryu" << endl;*/ return; } // 合流したのでノードを追加しない
            visited.insert(after_half_side_hash);
            
            int next_action_frame = node.frame;
            
            // この行動の決定のために使うであろう時間分のフレームを加算
            // (自分の時でないと正しくないが...)
            next_action_frame += (550 - 500 * field.countPuyos() / 78) * 60 / 1000;
            
            // 次の行動がとれるフレーム
            next_action_frame += plan.totalFrames();
            int pending_score = node.pending_score;
            
            if(rensa_result.chains > 0){ // 連鎖が起こるとき
                //cerr << rensa_result.chains << endl;
                next_action_frame += rensa_result.frames;
                
                // 必要連鎖数を超えない場合、必勝連鎖数を超えた場合は終了
                // firedを記録する必要はない
                if(next_action_frame >= node.pending_frame){
                    if(rensa_result.score < kMateScoreDist + g_minScore[1 - player]){ // 得点が足りないので負け
                        return;
                    }
                }
                
                fired_results.emplace_back(FiredResult{
                    rensa_result.chains,
                    rensa_result.score,
                    plan.decision(0),
                    node.first_decision,
                    node_index
                });

                DCHECK(examChain(fired_results.back().chain));
                
                if(rensa_result.chains >= chain_border){ return; }
                
                pending_score = max(0, pending_score - rensa_result.score); // おじゃまの量が減る
            }
            
            // おじゃまぷよの落下数
            int ojama = pending_score / SCORE_FOR_OJAMA;
            
            if(ojama && next_action_frame >= node.pending_frame){
                //CERR << "ojama" << endl;
                // 一列になっているおじゃまぷよを落とす
                next_action_frame += field.fallOjama(ojama / 6);
                
                if (!field.isEmpty(3, 12)){ // 3列目が埋まったので負け(以降を展開しない)
                    //CERR << "make" << endl;
                    //return;
                }
                
                //cerr << field;
                //cerr << (ojama % 6) << endl;
                // 端数のおじゃまぷよを落とす
                if(ojama % 6){
                    int ojama_position = decideOjamaComumn(ojama % 6, ojama_seed);
                    next_action_frame += fallFractionOjama(&field, ojama_position);
                }
                
                //cerr << field;
                // おじゃまが全部落ちたことにする
                pending_score = 0;
            }
            //CERR << field;
            if (!field.isEmpty(3, 12)){ // 3列目が埋まったので負け(以降を展開しない)
                //CERR << "make" << endl;
                return;
            }
            
            // 静的評価関数計算
            int score = node.score * 0.3 + evaluateField(field) * 0.7;
            
            // 状態キューに追加
            next_states.emplace_back(SoloNode{
                field,
                score,
                next_action_frame,
                0,
                pending_score,
                node.pending_frame,
                plan.decision(0),
                node.first_decision,
                node_index
            });
        };
        
        Plan::iterateAvailablePlans(node.field, {dropped}, 1, drop_callback);
    }
    
    tuple<Decision, int, int, int> soloSearch(const int player,
                                         const CoreField org_field,
                                         const KumipuyoSeq& seq,
                                         const int ojamaDice[],
                                         const int org_turn,
                                         const int org_frame,
                                         const int pending_score,
                                         const int pending_frame,
                                         const int chain_border){
        using node_t = SoloNode;
        using fired_result_t = FiredResult;
        
        //const int kBeamWidth = 200;
        
        //auto beamWidthFunc = [](int depth)->int{ return 50 + 250 * pow(0.965, depth - 1); }; // ビーム幅を減衰
        auto beamWidthFunc = [](int depth)->int{ return 80 + min(150.0, 50 * log((depth + 1) * 0.7)); }; // ビーム幅を増幅
        
        vector<node_t> stateQueue[N_MAX_SOLO_SEARCH_TURNS + 1];
        vector<fired_result_t> firedResult[N_MAX_SOLO_SEARCH_TURNS + 1];
        
        stateQueue[0].emplace_back(SoloNode{
            org_field,
            0,
            org_frame,
            0,
            pending_score,
            pending_frame,
            Decision(),
            Decision(),
            -1
        });
        
        const int search_turns = min(100000000, org_turn + max(10, min(50, ((6 * 13) - org_field.countPuyos()) / 2 + 4))); // 探索ターン数の上限を設定
        
        int max_chain = 0;
        int best_score = 0;
        Decision first_decision;
        
        for(int turn = org_turn; turn < search_turns; ++turn){
            int t = turn - org_turn;
            unordered_set<int64_t> visited; // 合流検知
            stateQueue[t + 1].reserve(stateQueue[t].size() * 22);
            firedResult[t].reserve(stateQueue[t].size() * 4);
            
            // キューに保存されている全ての状態に対して、次の状態を全て生成してキューに入れる
            for(int i = 0; i < (int)stateQueue[t].size(); ++i){
                //genNextSoloStates(stateQueue[t][i], i, seq.get(turn), chain_border, visited, stateQueue[t + 1], firedResult[t]);
                genNextSoloStates(player, stateQueue[t][i], i, seq.get(t), ojamaDice[0], chain_border, visited, stateQueue[t + 1], firedResult[t]);
            }
            
            // 最初のターンでfirst_decisionが設定されないのを直す
            if(turn == org_turn){
                for(auto& n : stateQueue[t + 1]){
                    n.first_decision = n.decision;
                    //CERR << "fired " << n.decision << ", " << endl;
                }
                for(auto& n : firedResult[t]){
                    n.first_decision = n.decision;
                }
            }
            // 展開する状態が無くなった
            if(stateQueue[t + 1].size() == 0){ break; }
            // 評価順に並べ替え
            sort(stateQueue[t + 1].begin(), stateQueue[t + 1].end(), [](const node_t& n0, const node_t& n1)->bool{
                return (n0.score > n1.score);
            });
            // ビーム幅をオーバーしたものは除外
            int beamWidth = beamWidthFunc(t);
            //cerr << beamWidth << endl;
            if((int)stateQueue[t + 1].size() > beamWidth){
                stateQueue[t + 1].erase(stateQueue[t + 1].begin() + beamWidth, stateQueue[t + 1].end());
            }
            //cerr << t << ", ";
            
            // 現時点での初手の評価をまとめる
            map<Decision, int> firstMap;
            for(const auto& n : stateQueue[t + 1]){
                firstMap[n.first_decision] += n.score;
            }
            
            // 現時点での最大連鎖
            for(const auto& n : firedResult[t]){
                
                if(n.chain > max_chain){
                    max_chain = n.chain;
                    best_score = n.score;
                    first_decision = n.first_decision;
                }
            }
            
            if(max_chain >= chain_border && firstMap.size() <= 1){
                // もう探索しても同じ手しかない
                CERR << "skipped in turn " << turn << endl;
                return make_tuple((*firstMap.begin()).first, max_chain, best_score, 0);
            }
            if(player == 1 && g_enemySearchNum){ // 相手視点での思考ストップ
                return make_tuple(first_decision, -1, -1, 0);
            }
        }
        
        // 探索後の処理
        vector<Decision> best = {Decision()};
        max_chain = 0;
        int fired_turn = 0;
        best_score = 0;
        /*vector<Decision> best[24];
         for(int i = 0; i < 24; ++i){
         best[i] = {Decision()};
         }
         
         for(int turn = org_turn; turn < org_turn + N_MAX_SOLO_SEARCH_TURNS; ++turn){
         int t = turn - org_turn;
         for(int i = 0; i < (int)firedResult[t].size(); ++i){
         const auto& fired = firedResult[t][i];
         if(fired.chain > max_chain){
         int first_decision_index = toDecisionIndex(fired.first_decision);
         
         max_chain = fired.chain;
         fired_turn = turn;
         
         best.clear();
         best.push_back(fired.decision);
         int prev_i = fired.previous_index;
         for(int tt = t; tt > 0; --tt){
         best.push_back(stateQueue[tt][prev_i].decision);
         prev_i = stateQueue[tt][prev_i].previous_index;
         }
         reverse(best[first_decision_index].begin(), best[first_decision_index].end());
         //CERR << turn << "(first = " << best << ")" << endl;
         
         DCHECK(fired.first_decision == best[first_decision_index][0]);
         }
         }
         }*/
        
        for(int turn = org_turn; turn < org_turn + N_MAX_SOLO_SEARCH_TURNS; ++turn){
            
            int t = turn - org_turn;
            std::sort(firedResult[t].begin(), firedResult[t].end(), [](const FiredResult& r0, const FiredResult& r1)->bool{
                return (r0.score > r1.score);
            });
            for(int i = 0; i < (int)firedResult[t].size(); ++i){
                const auto& fired = firedResult[t][i];
                //if(fired.chain > max_chain){
                if(fired.score > best_score){
                    //int first_decision_index = toDecisionIndex(fired.first_decision);
                    
                    max_chain = fired.chain;
                    best_score = fired.score;
                    fired_turn = turn;
                    
                    best.clear();
                    best.push_back(fired.decision);
                    int prev_i = fired.previous_index;
                    for(int tt = t; tt > 0; --tt){
                        best.push_back(stateQueue[tt][prev_i].decision);
                        prev_i = stateQueue[tt][prev_i].previous_index;
                    }
                    reverse(best.begin(), best.end());
                    //CERR << turn << "(first = " << best << ")" << endl;
                    
                    DCHECK(fired.first_decision == best[0]);
                }
            }
        }
        
        for(auto& d : best){
            CERR << d << ',';
        }CERR << endl;
        
        CERR << "decision " << best[0] << " chain = " << max_chain << "score = " << best_score << " in turn " << fired_turn << endl;
        return make_tuple(best[0], max_chain, best_score, fired_turn);
    }
    
    int soloSimulate(const int player,
                     const PlayerState& org_me,
                     const PlayerState& org_enemy,
                     const KumipuyoSeq& org_seq,
                     int org_turn,
                     int org_frame,
                     int chain_border){
        
        // ランダムにツモを決定
        KumipuyoSeq seq = org_seq;
        
        seq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(N_MAX_SOLO_SEARCH_TURNS, rand()));
        int ojamaDice[4] = {rand(), rand(), rand(), rand()};
        
        CERR << seq.toString() << endl;
        
        // 初期状態の抽出
        int org_pending_score = org_enemy.isRensaOngoing() ? scoreForOjama(org_me.totalOjama(org_enemy)) : 0;
        int org_pending_frame = org_enemy.isRensaOngoing() ? org_enemy.rensaFinishingFrameId() : 9999999;
        
        //cerr << "pending score = " << org_pending_score << " in frame " << org_pending_frame << endl;
        // 探索を行う
        auto result = soloSearch(player, org_me.field, seq, ojamaDice, org_turn, org_frame, org_pending_score, org_pending_frame, chain_border);

        g_soloSearchResults[player].pushResult(SoloSearchResult{
            seq, // sequence
            get<3>(result), // turn
            get<1>(result), // chain
            get<2>(result), // score
            get<0>(result), // decision
        });
        if(get<1>(result) == 0){
            return -1;
        }
        return 0;
    }
    
    int soloSimulateThread(const int thread_id,
                           const int player,
                           const PlayerState& org_me,
                           const PlayerState& org_enemy,
                           const KumipuyoSeq& org_seq,
                           int org_turn,
                           int org_frame){
        
        UNUSED_VARIABLE(thread_id);
        
        int chain_border = min(14, org_me.field.countColorPuyos() / 5 + 4);
        //cerr << ClockMS::now() << endl;
        while(player == 1 || (ClockMS::now() < g_limitTime && chain_border > 0)){
            //cerr << ClockMS::now() << endl;
            int ret = soloSimulate(player, org_me, org_enemy, org_seq, org_turn, org_frame, chain_border);
            
            ret += 1;
            
            if(player == 1 && g_enemySearchNum){ // 相手視点での思考ストップ
                cerr << "stopped" << endl;
                g_enemySearchNum -= 1;
                break;
            }
        }
        return 0;
    }
}

#endif // PUYOAI_YURICAT_SOLOSEARCH_HPP_
