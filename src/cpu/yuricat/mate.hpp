/*
 mate.hpp
 yuricat(Katsuki Ohto)
 */

// 詰み探索
// ただし端数のおじゃまぷよが絡む場合があるので、確率を返す

#ifndef PUYOAI_YURICAT_MATE_HPP_
#define PUYOAI_YURICAT_MATE_HPP_

#include "puyo.hpp"

namespace Yuricat{
    
    double isOjamaMate(const CoreField& f, const KumipuyoSeq& seq, int frame_now, int frame_ojama, int pending_score){
        // |frame|に降るおじゃまぷよ|pending_score|点分によって詰むか調べる
        
        double mate_rate = 1;
        Plan::iterateAvailablePlans(f, seq, 1, [frame_now, frame_ojama, pending_score, &mate_rate](const RefPlan& plan) {
            int frame = frame_now + plan.totalFrames();
            const CoreField& after_field = plan.field();
            if(frame > frame_ojama){
                // ここでおじゃまが降る
                int new_pending_score = pending_score;
                if(plan.isRensaPlan()){
                    new_pending_score -= plan.rensaResult().score;
                }
                if(pending_score > 0){
                    int ojama = pending_score / SCORE_FOR_OJAMA;
                    int lines = ojama / 6;
                    int new_height3 = after_field.height(3) + lines;
                    if(new_height3 < 12){
                        if(new_height3 == 11 && (lines % 6)){
                            // 端数によって即死の可能性あり
                            mate_rate = min(mate_rate, (lines % 6) / 6.0);
                        }else{
                            // とりあえず死にはしない
                            //return 0.0;
                        }
                        // 即死ではないがやばそうな場合は後で考える...
                        //after_field.dropOjama();
                        
                    }else{
                        // それ以外は即死
                    }
                }
            }
        });
        
        return mate_rate;
    }
    /*
    int solveRensaScore(int act){
        const int me = act;
        const int opp = 1 - act;
        int i0 = 0, i1 = 0;
        FrameScore guarded; // 効かない最大(frame, score)
        while(1){
            FrameScore sc = g_totalRensaFrameScore[me][i0];
            // この攻撃に耐えられるか
            // より前のフレームで
            auto lover_bound(
            if(){
                
            }else{
                
            }
        }
    }*/
}

#endif // PUYOAI_YURICAT_MATE_HPP_
