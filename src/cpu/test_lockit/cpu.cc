#include "cpu.h"

#include <gflags/gflags.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/puyo_color.h"
#include "field.h"
#include "util.h"

DEFINE_string(io_log, "", "Specify prefix of I/O log if you want to take log.");

using namespace std;

namespace test_lockit {

TestLockitAI::TestLockitAI(const cpu::Configuration& config)
    : config(config)
    , coma(config)
    , coma2x(config)
{
    r_player[0].ref();
    r_player[1].ref();
    coma.ref();
    coma2x.ref();
    if (!FLAGS_io_log.empty()) {
        // Existing logs are discarded.
        play_log_.open(FLAGS_io_log, std::fstream::out | std::ofstream::trunc);
    }
}

void parseRequest(const FrameRequest& request, READ_P* p1, READ_P* p2, COMAI_HI* coo)
{
    p1->id = request.frameId;
    if (request.shouldInitialize()) {
        p1->ref();
        p2->ref();
        coo->ref();
    }

    {
        const KumipuyoSeq& seq = request.myPlayerFrameRequest().kumipuyoSeq;
        for (int i = 0; i < 6; ++i) {
            p1->tsumo[i] = PuyoColor::IRON;
        }
        if (seq.size() >= 1) {
            p1->tsumo[0] = seq.axis(0);
            p1->tsumo[1] = seq.child(0);
        }
        if (seq.size() >= 2) {
            p1->tsumo[2] = seq.axis(1);
            p1->tsumo[3] = seq.child(1);
        }
        if (seq.size() >= 3) {
            p1->tsumo[4] = seq.axis(2);
            p1->tsumo[5] = seq.child(2);
        }
    }
    {
        const KumipuyoSeq& seq = request.enemyPlayerFrameRequest().kumipuyoSeq;
        for (int i = 0; i < 6; ++i) {
            p2->tsumo[i] = PuyoColor::IRON;
        }
        if (seq.size() >= 1) {
            p2->tsumo[0] = seq.axis(0);
            p2->tsumo[1] = seq.child(0);
        }
        if (seq.size() >= 2) {
            p2->tsumo[2] = seq.axis(1);
            p2->tsumo[3] = seq.child(1);
        }
        if (seq.size() >= 3) {
            p2->tsumo[4] = seq.axis(2);
            p2->tsumo[5] = seq.child(2);
        }
    }

    {
        const PlainField& pf = request.myPlayerFrameRequest().field;
        for (int x = 0; x < 6; ++x) {
            for (int y = 0; y < 14; ++y) {
                p1->field[x][y] = pf.color(x + 1, y + 1);
            }
        }
    }
    {
        const PlainField& pf = request.enemyPlayerFrameRequest().field;
        for (int x = 0; x < 6; ++x) {
            for (int y = 0; y < 14; ++y) {
                p2->field[x][y] = pf.color(x + 1, y + 1);
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
    }
}

FrameResponse sendmes(READ_P* p1, COMAI_HI* coo)
{
    stringstream ss;
    ss << "you="
       << "x-r:" << p1->te_x << p1->te_r << " "
       << "nok:" << coo->m_aite_hakka_nokori;

    return FrameResponse(p1->id, Decision(p1->te_x, p1->te_r), ss.str());
}

bool isTsumoValid(PuyoColor tsumo[6])
{
    // HACK(mayah): Check only [2, 4).
    // [0, 2) might be EMPTY before the game has started.
    // [4, 6) might be EMPTY before WNEXT come.
    for (int i = 2; i < 4; ++i) {
        PuyoColor c = tsumo[i];
        if (!isNormalColor(c))
            return false;
    }

    return true;
}

FrameResponse TestLockitAI::playOneFrame(const FrameRequest& request)
{
    int saidaiten, tmp;
    int coma2x_sc[22] {};

    parseRequest(request, &r_player[0], &r_player[1], &coma);

    if (!isTsumoValid(r_player[0].tsumo) || !isTsumoValid(r_player[1].tsumo)) {
        return FrameResponse(request.frameId);
    }

    FrameResponse response(request.frameId);

    // 相手攻撃判断
    if (r_player[1].set_puyo == 1) {
        r_player[1].set_puyo_once = 1;
        r_player[1].fall();
        r_player[1].keep_score = r_player[1].score;
        if (coma.aite_attack_start(r_player[1].field, r_player[1].zenkesi, coma.m_aite_hakkaji_score, r_player[0].id)) {
            r_player[1].zenkesi = 0;
        }
    }

    // 相手全消し,全消し消化判断
    if (r_player[1].rensa_end == 1 && isTLFieldEmpty(r_player[1].field)) {
        r_player[1].zenkesi = 1;
    }

    // 相手凝視
    if ((r_player[1].act_on == 1) && (r_player[1].set_puyo_once == 1)) {
        r_player[1].set_puyo_once = 0;
        coma.aite_rensa_end();
        coma.aite_hyouka(r_player[1].field, r_player[1].tsumo);
    }
    if (r_player[0].set_puyo == 1) {
        r_player[0].set_puyo_once = 1;
        r_player[0].keep_score = r_player[0].score;
    }
    if (r_player[0].rensa_end == 1) { // 全消し,全消し消化判断
        r_player[0].field_kioku();
        if (isTLFieldEmpty(r_player[0].field)) {
            r_player[0].zenkesi = 1; // 得点系のため一旦out
        }
    }

    if (r_player[0].act_on == 1 && r_player[0].set_puyo_once == 1) {
        r_player[0].set_puyo_once = 0;
        r_player[0].setti_12();
        if (coma.m_hukks == 0 || r_player[0].field_hikaku() > 0) { // 開幕のm_hukks==0では思考を短くする？
            coma.pre_hyouka(r_player[0].field, r_player[0].tsumo, r_player[0].zenkesi, r_player[1].field,
                            r_player[1].zenkesi, 1);
        }
        coma.aite_attack_nokori(r_player[1].field,
                                r_player[0].id); // 情報が更新されないため、現構成ではうまく機能しない
        coma.hyouka(r_player[0].field, r_player[0].tsumo, r_player[0].zenkesi, r_player[1].field, r_player[1].zenkesi);

        // 2x hyouka
        int field_kosuu = 0;
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < kHeight; ++j) {
                if (r_player[0].field[i][j] != PuyoColor::EMPTY) {
                    ++field_kosuu;
                }
            }
        }
        // If we don't use 2x-hyouka, set field_kosuu to 0 so that 2x-hyouka is not used.
        if (!config.uses_2x_hyouka) {
            field_kosuu = 0;
        }

        PuyoColor field2x[6][kHeight] {};
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 14; ++j) {
                field2x[i][j] = (j + 4 < kHeight) ? r_player[0].field[i][j + 4] : PuyoColor::EMPTY;
            }
        }

        if (field_kosuu > 24 && field_kosuu < 56) {
            coma2x.hyouka(field2x, r_player[0].tsumo, r_player[0].zenkesi, r_player[1].field, r_player[1].zenkesi);
        }

        tmp = 0;
        r_player[0].te_x = 0;
        r_player[0].te_r = 0;
        saidaiten = coma.m_para[0];

        for (int i = 0; i < 22; i++) {
            if ((field_kosuu > 24) && (field_kosuu < 56)) {
                coma2x_sc[i] = coma2x.m_para[i] + coma.m_para[i];
            } else {
                coma2x_sc[i] = coma.m_para[i];
            }
        }
        saidaiten = coma2x_sc[0];
        for (int i = 1; i < 22; i++) {
            if (saidaiten < coma2x_sc[i]) {
                tmp = i;
                saidaiten = coma2x_sc[i];
            }
        }

        if (tmp < 6) {
            r_player[0].te_x = tmp + 1;
            r_player[0].te_r = 0;
        } else if (tmp < 12) {
            r_player[0].te_x = tmp - 6 + 1;
            r_player[0].te_r = 2;
        } else if (tmp < 17) {
            // In case r=3, we use 2 <= x <= 6.
            r_player[0].te_x = tmp - 12 + 2;
            r_player[0].te_r = 3;
        } else if (tmp < 22) {
            // In case r=1, we use 1 <= x <= 5.
            r_player[0].te_x = tmp - 17 + 1;
            r_player[0].te_r = 1;
        }
        if (r_player[0].setti_puyo()) {
            r_player[0].zenkesi = 0;
        }
        response = sendmes(&r_player[0], &coma);
    } // p1 act_once

    if (r_player[0].nex_on == 1) { // 事前手決めスタート
        if (coma.m_hukks != 0) {
            coma.pre_hyouka(r_player[0].yosou_field, r_player[0].tsumo + 2, r_player[0].zenkesi, r_player[1].field,
                            r_player[1].zenkesi, 0);
        }
    } // 開幕のm_hukks==0はこちらはひっかからない？

    if (play_log_.is_open()) {
        std::string request_str = request.toString();
        if (request_str.substr(request_str.find(' ') + 1) != last_log_) {
            play_log_ << request_str << "\n" << response.toString() << std::endl;
            last_log_ = request_str.substr(request_str.find(' ') + 1);
        }
    }

    return response;
}

void TestLockitAI::runTest(const std::string& filename)
{
    DCHECK(!filename.empty());
    std::ifstream ifs(filename);

    std::string input, expect;
    int count = 0;
    while (std::getline(ifs, input)) {
        std::getline(ifs, expect);

        FrameResponse response = playOneFrame(FrameRequest::parsePayload(input.c_str(), input.size()));
        std::string output = response.toString();
        if (expect != output) {
            std::cerr << "EXPECT: " << expect << "\n"
                      << "ACTUAL: " << output << "\n"
                      << "for :" << input << std::endl;
            return;
        }
        if (++count % 1000 == 0) {
            std::cerr << "Passed " << count << " cases\n";
        }
    }
    std::cerr << "Passed all " << count << " cases.\n";
}

} // namespace test_lockit
