/*
 evaluation.hpp
 yuricat(Katsuki Ohto)
 */

#ifndef PUYOAI_YURICAT_EVALUATION_HPP_
#define PUYOAI_YURICAT_EVALUATION_HPP_

#include "core/rensa/rensa_detector.h"

namespace Yuricat{
    
    using ChainResult = tuple<int, int>;
    
    inline void detectMainChain(const CoreField& field, ChainResult *const best, ChainResult *const second){
        *best = make_tuple(0, 0);
        *second = make_tuple(0, 0);
        
        bool prohibits[FieldConstant::MAP_WIDTH] = {0};
        
        auto complement_callback = [best, second]
        (CoreField&& complemented_field, const ColumnPuyoList& puyo_list){
            int ignition_y = -1;
            for(int x = 1; x <= FieldConstant::WIDTH; ++x){
                if(puyo_list.sizeOn(x) > 0){
                    ignition_y = complemented_field.height(x); // 補完された盤面の高さ
                }
            }
            
            RensaResult rensa = complemented_field.simulate();
            auto sample = make_tuple(rensa.chains, ignition_y);
            if(sample > *second){
                if(sample > *best){
                    *second = *best;
                    *best = sample;
                }else{
                    *second = sample;
                }
            }
        };
        
        // 保持している連鎖を調べる
        RensaDetector::detectByDropStrategy(field, prohibits, PurposeForFindingRensa::FOR_FIRE, 2, 13, complement_callback);
    }
    
    template<class field_t>
    int evaluateField(const field_t& field/*, const Kumipuyo& kumipuyo*/){
        
        // staticな局面評価
        // takaptさんのAIを参考に
        
        int score = 0;
        
        // 現時点での連鎖力
        ChainResult best, second;
        detectMainChain(field, &best, &second);
        
        // 現時点での連鎖の強さ
        if (get<0>(best) >= 2){
            score += get<0>(best) * 1000;
            // 最高発火点が高い方がいい
            score += 10 * get<1>(best);
            // 2番目の発火候補に対する得点
            /*if (get<0>(second) >= 2){
                score += get<0>(second) * 10;
                // 最高発火点が高い方がいい
                score += 1 * get<1>(second);
            }*/
        }
        // 1手で打てる連鎖の大きさ
        
        // U字型
        int sum_height = 0;
        for(int i = 1; i <= FieldConstant::WIDTH; ++i){
            sum_height += field.height(i);
        }
        double avg_height = sum_height / 6.0;
        
        double u_score = 0;
        static const double diff[] = {0, +2, 0, -2, -2, 0, +2, 0};
        for(int x = 1; x <= FieldConstant::WIDTH; ++x){
            u_score -= fabs((field.height(x) - avg_height) - diff[x]);
        }
        
        score += u_score * 60;
        
        // 3列目の近くに逃げ場が残っているかどうか
        int coef_score = 0;
        static const int coef[] = {0, 1, 3, 0, 3, 2, 1, 0};
        for (int x = 1; x <= FieldConstant::WIDTH; ++x){
            coef_score -= coef[x] * !field.isEmpty(x, 13) * 5;
        }
        score += coef_score;
        
        // 3列目が高いとやばい
        if(field.height(3) == 11){
            score -= 600;
        }else if(field.height(3) == 10){
            score -= 100;
        }else if(field.height(3) == 9){
            score -= 30;
        }
        
        // 高低差がありすぎることの減点
        int min_height = 9999;
        int max_height = 0;
        for(int x = 1; x <= FieldConstant::WIDTH; ++x){
            min_height = min(min_height, field.height(x));
            max_height = max(max_height, field.height(x));
        }
        score -= (max_height - min_height) * 2;
        
        // 谷の減点
        int max_valley = 0;
        max_valley = max(max_valley, max(0, field.height(2) - field.height(1)));
        for(int x = 1 + 1; x <= FieldConstant::WIDTH - 1; ++x){
            int v = max(0, min(field.height(x - 1), field.height(x + 1)) - field.height(x));
            max_valley = max(max_valley, v);
        }
        max_valley = max(max_valley, max(0, field.height(5) - field.height(6)));
        if(max_valley > 1){
            score -= 100 * max_valley;
        }
        
        // おじゃまぷよの減点
        score -= field.countColor(PuyoColor::OJAMA) * 3;
        
        return score;
    }
    /*
    template<class field_t>
    int evalateFields(const field_t& field0, const field_t& field1){
        return evaluateField(field0) - evaluateField(field1);
    }
    
    template<class field_t, class plan_t>
    int evaluatePlanFast(const field_t& field, const plan_t& plan){
        // 行動価値関数(局面遷移をせずに求める)
        
        return 0;
    }
    */
    template<class field_t, class plan_t>
    int evaluatePlan(const field_t& field, const plan_t& plan){
        // 行動価値関数(重めでもいい)
        UNUSED_VARIABLE(field);
        if (plan.isRensaPlan()) {
            //RensaResult rensa = plan.rensaResult();
            //int chain = rensa.chains; // 連鎖数
        }
        //int chigiri = plan.numChigiri(); // ちぎりの数
        
        const CoreField& after_field = plan.field(); // 後場
        
        return evaluateField(after_field);
    }
    
}

#endif // PUYOAI_YURICAT_EVALUATION_HPP_
