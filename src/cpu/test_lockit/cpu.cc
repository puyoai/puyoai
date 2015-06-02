#include <iostream>
#include <string>
#include <sstream>

#include "core/puyo_color.h"
#include "core/frame_request.h"
#include "core/frame_response.h"

#include "color.h"
#include "cpu.h"
#include "util.h"

using namespace std;

namespace test_lockit {

namespace {
int count_tsumo_2p = 0;

int count_all[20] = { 0 };
int checks = 0;
}

TestLockitAI::TestLockitAI()
{
    r_player[0].ref();
    r_player[1].ref();
    coma.ref();
    coma2x.ref();
}

void parseRequest(const FrameRequest& request, READ_P* p1, READ_P* p2, COMAI_HI* coo)
{
    p1->id = request.frameId;
    if (request.shouldInitialize()) {
        p1->ref();
        p2->ref();
        coo->ref();
        memset(count_all, 0, sizeof(count_all));
        checks = 0;
    }

    {
        const KumipuyoSeq& seq = request.myPlayerFrameRequest().kumipuyoSeq;
        p1->tsumo[0] = toTLColor(seq.axis(0));
        p1->tsumo[1] = toTLColor(seq.child(0));
        p1->tsumo[2] = toTLColor(seq.axis(1));
        p1->tsumo[3] = toTLColor(seq.child(1));
        if (seq.size() >= 3) {
            p1->tsumo[4] = toTLColor(seq.axis(2));
            p1->tsumo[5] = toTLColor(seq.child(2));
        } else {
            p1->tsumo[4] = TLColor::TL_EMPTY;
            p1->tsumo[5] = TLColor::TL_EMPTY;
        }
    }
    {
        const KumipuyoSeq& seq = request.enemyPlayerFrameRequest().kumipuyoSeq;
        p2->tsumo[0] = toTLColor(seq.axis(0));
        p2->tsumo[1] = toTLColor(seq.child(0));
        p2->tsumo[2] = toTLColor(seq.axis(1));
        p2->tsumo[3] = toTLColor(seq.child(1));
        if (seq.size() >= 3) {
            p2->tsumo[4] = toTLColor(seq.axis(2));
            p2->tsumo[5] = toTLColor(seq.child(2));
        } else {
            p2->tsumo[4] = 0;
            p2->tsumo[5] = 0;
        }
    }

    {
        const PlainField& pf = request.myPlayerFrameRequest().field;
        for (int x = 0; x < 6; ++x) {
            for (int y = 0; y < 12; ++y) {
                p1->field[x][y] = toTLColor(pf.color(x + 1, y + 1));
            }
        }
    }
    {
        const PlainField& pf = request.enemyPlayerFrameRequest().field;
        for (int x = 0; x < 6; ++x) {
            for (int y = 0; y < 12; ++y) {
                p2->field[x][y] = toTLColor(pf.color(x + 1, y + 1));
            }
        }
    }

    {
        const UserEvent& event = request.myPlayerFrameRequest().event;
        p1->act_on = event.decisionRequest || event.decisionRequestAgain;
        p1->nex_on = event.wnextAppeared;
        p1->set_puyo = event.grounded;
        p1->rensa_end = event.puyoErased;
        p1->score = request.myPlayerFrameRequest().score;
    }

    {
        const UserEvent& event = request.enemyPlayerFrameRequest().event;
        p2->act_on = event.decisionRequest || event.decisionRequestAgain;
        p2->nex_on = event.wnextAppeared;
        p2->set_puyo = event.grounded;
        p2->rensa_end = event.puyoErased;
        p2->score = request.enemyPlayerFrameRequest().score;
    }

    if (request.gameResult != GameResult::PLAYING) {
        p1->ref();
        p2->ref();
        coo->ref();
        memset(count_all, 0, sizeof(count_all));
        checks = 0;
    }
}

FrameResponse sendmes(READ_P* p1, READ_P* /*p2*/, COMAI_HI* coo)
{
    stringstream ss;
    ss << "you="
       << "x-r:" << p1->te_x << p1->te_r << " "
       << "nxt:" << count_all[4] << " "
       << "set:" << count_all[5] << " "
       << "end:" << count_all[6] << " "
       << "act:" << count_all[7] << " "
       << "a-m:" << count_all[9] << " "
       << "ret:" << count_all[10] << " "
       << "nok:" << coo->aite_hakka_nokori;

    return FrameResponse(p1->id, Decision(p1->te_x, p1->te_r), ss.str());
}

FrameResponse TestLockitAI::playOneFrame(const FrameRequest& request)
{
    int saidaiten, tmp;
    int coma2x_sc[22] {};
//    int field2x[6][18] {};

    parseRequest(request, &r_player[0], &r_player[1], &coma);
    //	  if(r_player[0].nex_on==1)checks=1;
    //	  if((r_player[0].rensa_end==1)&&(r_player[0].tsumo[0]!=0)&&(r_player[0].tsumo[4]==0))checks=1;

    FrameResponse response(request.frameId);

    if (checks == 0) {

        if (r_player[0].nex_on == 1) {
            count_all[0]++;
        }
        if (r_player[1].set_puyo == 1) { //相手攻撃判断
            count_all[1]++;
            r_player[1].set_puyo_once = 1;
            r_player[1].fall();
            r_player[1].keep_score = r_player[1].score;
            //			coma.aite_attack_start(r_player[1].field, r_player[1].zenkesi,
            //r_player[1].score);
            //			coma.aite_attack_start(r_player[1].field, r_player[1].zenkesi,
            //coma.aite_hakkaji_score, r_player[0].id);
            if (coma.aite_attack_start(r_player[1].field, r_player[1].zenkesi, coma.aite_hakkaji_score,
                                       r_player[0].id)) { //スコア取得バグ
                r_player[1].zenkesi = 0;
            }
        }
        if (r_player[1].rensa_end == 1) { //相手全消し,全消し消化判断
            count_all[2]++;
            //			if(r_player[1].score>r_player[1].keep_score+39){
            //				r_player[1].zenkesi = 0;
            //			}
            if ((r_player[1].field[0][0] == 0) && (r_player[1].field[1][0] == 0) && (r_player[1].field[2][0] == 0)
                && (r_player[1].field[3][0] == 0) && (r_player[1].field[4][0] == 0)
                && (r_player[1].field[5][0] == 0)) {
                r_player[1].zenkesi = 1; //得点系のため一旦out
            }
        }
        if ((r_player[1].act_on == 1) && (r_player[1].set_puyo_once == 1)) { //相手凝視
            count_all[3]++;
            r_player[1].set_puyo_once = 0;
            coma.aite_rensa_end();
            if (r_player[1].tsumo[0] == 0) {
                r_player[1].tsumo[0] = 1;
                count_all[8]++;
            }
            if (r_player[1].tsumo[1] == 0)
                r_player[1].tsumo[1] = 1;
            if (r_player[1].tsumo[2] == 0) {
                r_player[1].tsumo[2] = 1;
                count_all[8]++;
            }
            if (r_player[1].tsumo[3] == 0)
                r_player[1].tsumo[3] = 1;
            coma.aite_hyouka(r_player[1].field, r_player[1].tsumo[0], r_player[1].tsumo[1], r_player[1].tsumo[2],
                             r_player[1].tsumo[3]);
        }
        if (r_player[0].set_puyo == 1) {
            count_all[5]++;
            r_player[0].set_puyo_once = 1;
            r_player[0].keep_score = r_player[0].score;
        }
        if (r_player[0].rensa_end == 1) { //全消し,全消し消化判断
            count_all[6]++;
            //			r_player[0].set_puyo_once=1;
            r_player[0].field_kioku();
            //			if(r_player[0].score>r_player[0].keep_score+39){
            //				r_player[0].zenkesi = 0;
            //			}
            if ((r_player[0].field[0][0] == 0) && (r_player[0].field[1][0] == 0) && (r_player[0].field[2][0] == 0)
                && (r_player[0].field[3][0] == 0) && (r_player[0].field[4][0] == 0)
                && (r_player[0].field[5][0] == 0)) {
                r_player[0].zenkesi = 1; //得点系のため一旦out
            }
        }
        if ((r_player[0].act_on == 1) && (r_player[0].set_puyo_once == 1)) {
            count_all[7]++;
            r_player[0].set_puyo_once = 0;
            r_player[0].setti_12();
            if (r_player[0].tsumo[0] == 0) {
                r_player[0].tsumo[0] = 1;
                count_all[9]++;
            }
            if (r_player[0].tsumo[1] == 0)
                r_player[0].tsumo[1] = 1;
            if (r_player[0].tsumo[2] == 0) {
                r_player[0].tsumo[2] = 1;
                count_all[9]++;
            }
            if (r_player[0].tsumo[3] == 0)
                r_player[0].tsumo[3] = 1;
            if ((coma.hukks == 0) || (r_player[0].field_hikaku() > 0)) { //開幕のhukks==0では思考を短くする？
                if (coma.hukks != 0)
                    count_all[10]++;
                coma.pre_hyouka(r_player[0].field, r_player[0].tsumo[0], r_player[0].tsumo[1], r_player[0].tsumo[2],
                                r_player[0].tsumo[3], r_player[0].zenkesi, r_player[1].field, r_player[1].zenkesi,
                                1);
            }
            coma.aite_attack_nokori(r_player[1].field,
                                    r_player[0].id); //情報が更新されないため、現構成ではうまく機能しない
            coma.hyouka(r_player[0].field, r_player[0].tsumo[0], r_player[0].tsumo[1], r_player[0].tsumo[2],
                        r_player[0].tsumo[3], r_player[0].zenkesi, r_player[1].field, r_player[1].zenkesi);

            // 2x hyouka
            int field_kosuu = 0;
            for (int i = 0; i < 6; ++i) {
                for (int j = 0; j < kHeight; ++j) {
                    if (r_player[0].field[i][j] != 0) {
                        ++field_kosuu;
                    }
                }
            }
            // If we don't use 2x-hyouka, set field_kosuu to 0 so that 2x-hyouka is not used.
            if (!uses_2x_hyouka) {
                field_kosuu = 0;
            }

            int field2x[6][kHeight] {};
            for (int i = 0; i < 6; ++i) {
                for (int j = 0; j < 14; ++j) {
                    field2x[i][j] = (j + 4 < kHeight) ? r_player[0].field[i][j+4] : 0;
                }
            }

            if (field_kosuu > 24 && field_kosuu < 56) {
              coma2x.hyouka(field2x, r_player[0].tsumo[0], r_player[0].tsumo[1], r_player[0].tsumo[2], r_player[0].tsumo[3], r_player[0].zenkesi, r_player[1].field, r_player[1].zenkesi);
            }

            count_tsumo_2p++;
            tmp = 0;
            r_player[0].te_x = 0;
            r_player[0].te_r = 0;
            saidaiten = coma.para[0];

            for (int i = 0; i < 22; i++) {
                if ((field_kosuu>24) && (field_kosuu<56)){
                    coma2x_sc[i] = coma2x.para[i] + coma.para[i];
                } else {
                    coma2x_sc[i] = coma.para[i];
                }
            }
            saidaiten = coma2x_sc[0];
            for (int i = 1; i < 22; i++) {
                if (saidaiten<coma2x_sc[i]){
                    tmp = i;
                    saidaiten = coma2x_sc[i];
                }
            }

            if (tmp < 6) {
                r_player[0].te_x = tmp + 1;
                r_player[0].te_r = 0;
            } else if (tmp < 11) {
                r_player[0].te_x = tmp - 5;
                r_player[0].te_r = 1;
            } else if (tmp < 17) {
                r_player[0].te_x = tmp - 10;
                r_player[0].te_r = 2;
            } else if (tmp < 22) {
                r_player[0].te_x = tmp - 15;
                r_player[0].te_r = 3;
            }
            //			r_player[0].setti_puyo();
            if (r_player[0].setti_puyo()) {
                r_player[0].zenkesi = 0;
            }
            response = sendmes(&r_player[0], &r_player[1], &coma);
        } // p1 act_once

        if (r_player[0].nex_on == 1) { //事前手決めスタート
            //		if(r_player[0].set_puyo==1){
            count_all[4]++;
            if (r_player[0].tsumo[2] == 0) {
                r_player[0].tsumo[2] = 1;
                count_all[9]++;
            }
            if (r_player[0].tsumo[3] == 0)
                r_player[0].tsumo[3] = 1;
            if (r_player[0].tsumo[4] == 0) {
                r_player[0].tsumo[4] = 1;
                count_all[9]++;
            }
            if (r_player[0].tsumo[5] == 0)
                r_player[0].tsumo[5] = 1;
            if (coma.hukks != 0)
                coma.pre_hyouka(r_player[0].yosou_field, r_player[0].tsumo[2], r_player[0].tsumo[3],
                                r_player[0].tsumo[4], r_player[0].tsumo[5], r_player[0].zenkesi, r_player[1].field,
                                r_player[1].zenkesi, 0);
            //			coma.pre_hyouka(r_player[0].field, r_player[0].tsumo[2], r_player[0].tsumo[3],
            //r_player[0].tsumo[4], r_player[0].tsumo[5], r_player[0].zenkesi, r_player[1].field,
            //r_player[1].zenkesi);
        } //開幕のhukks==0はこちらはひっかからない？
        //		if(r_player[0].act_on==1){
        //		sendmes(&r_player[0], &r_player[1], &coma); // 毎ターン発行しないほうがよい
        //		}
        //		if(r_player[0].act_on==1){
        //			r_player[0].te_x = 3;
        //			r_player[0].te_r = 0;
        //		}
    } // checks

    return response;
}

}  // namespace test_lockit

