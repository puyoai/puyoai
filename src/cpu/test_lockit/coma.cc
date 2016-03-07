#include "coma.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "base/base.h"
#include "core/puyo_color.h"

#include "field.h"
#include "rensa_result.h"
#include "template.h"

namespace test_lockit {

namespace {

int g_chainhyk[22][22][221][EE_SIZE] {};
int g_poihyo[22][22][221][EE_SIZE] {};
int g_score_hukasa[22][22][221] {};

const PuyoColor kTsumoPatterns[][2] = {
    {PuyoColor::RED, PuyoColor::RED},
    {PuyoColor::RED, PuyoColor::BLUE},
    {PuyoColor::RED, PuyoColor::YELLOW},
    {PuyoColor::RED, PuyoColor::GREEN},
    {PuyoColor::BLUE, PuyoColor::BLUE},
    {PuyoColor::BLUE, PuyoColor::YELLOW},
    {PuyoColor::BLUE, PuyoColor::GREEN},
    {PuyoColor::YELLOW, PuyoColor::YELLOW},
    {PuyoColor::YELLOW, PuyoColor::GREEN},
    {PuyoColor::GREEN, PuyoColor::GREEN},
};

bool isMatchIndexAndColors(int id, PuyoColor color[]) {
  // PuyoColor::IRON is used for a place holder.
  if (color[0] == PuyoColor::IRON || color[1] == PuyoColor::IRON)
    return true;
  if (id == 220)
    return true;
  if (id >= 221 || id < 0)
    return false;

  id /= 22;
  return kTsumoPatterns[id][0] == color[0] && kTsumoPatterns[id][1] == color[1];
}

int isChigiri(int cells[]) {
    if (cells[0] != cells[2] && cells[1] != cells[3])
        return 1;
    else
        return 0;
}

PuyoColor toValidPuyoColor(PuyoColor c)
{
    // HACK(peria): Convert an unknown color to RED to avoid out-of-range.
    // NOTE: We can use other colors insted, but use only RED to keep code simple.
    if (c == PuyoColor::IRON)
        return PuyoColor::RED;
    return c;
}

void UpdateAccessibility(PuyoColor ba[6][kHeight], Check point2[6][12])
{
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 12; j++) {
            point2[i][j] = Check::Unknown;
            if (!isNormalColor(ba[i][j]))
                continue;

            bool isAboveEmpty = (j != 11 && ba[i][j + 1] == PuyoColor::EMPTY);
            bool isRightEmpty = (i != 5 && ba[i + 1][j] == PuyoColor::EMPTY);
            bool isLeftEmpty = (i != 0 && ba[i - 1][j] == PuyoColor::EMPTY);
            if (isAboveEmpty) {
                if (isRightEmpty && (i != 4 || ba[3][11] == PuyoColor::EMPTY)) {
                    point2[i][j] = Check::ColorWithEmptyUR;
                    point2[i][j + 1] = Check::Empty;
                    break;
                }
                if (isLeftEmpty && (i != 5 || ba[3][11] == PuyoColor::EMPTY)) {
                    point2[i][j] = Check::ColorWithEmptyUL;
                    point2[i][j + 1] = Check::Empty;
                    break;
                }
                if ((i != 0 || ba[1][11] == PuyoColor::EMPTY)
                    && (i != 4 || ba[3][11] == PuyoColor::EMPTY)
                    && (i != 5 || (ba[3][11] == PuyoColor::EMPTY && ba[4][11] == PuyoColor::EMPTY))) {
                    point2[i][j] = Check::ColorWithEmptyU;
                    point2[i][j + 1] = Check::Empty;
                    break;
                }
            } else {
                if (isRightEmpty && isLeftEmpty && ba[i][11] == PuyoColor::EMPTY) {
                    point2[i][j] = Check::ColorWithEmptyLR;
                    continue;
                }
                if (isRightEmpty
                    && (i != 4 || (ba[3][11] == PuyoColor::EMPTY && ba[4][11] == PuyoColor::EMPTY))
                    && (i != 3 || ba[3][11] == PuyoColor::EMPTY)) {
                    point2[i][j] = Check::ColorWithEmptyL;
                    continue;
                }
                if (isLeftEmpty
                    && (i != 1 || ba[1][11] == PuyoColor::EMPTY)
                    && (i != 5 || ba[3][11] == PuyoColor::EMPTY)) {
                    point2[i][j] = Check::ColorWithEmptyR;
                    continue;
                }
            }
        }
    }
}

}  // namespace

COMAI_HI::COMAI_HI(const cpu::Configuration& config) :
    config(config)
{
    ref();
}

COMAI_HI::~COMAI_HI()
{
}

void COMAI_HI::ref()
{
    m_cchai = 0;
    m_hukks = 0;
    m_conaa = 0;
    m_nexaa = 0;
    m_maxchais = 0;
    for (int i = 0; i < 22; i++) {
        m_para[i] = 0;
    }
    // NOTE(peria): [8] means (x,r)=(3,2)
    m_para[8] = 1;
    m_myf_kosuu = 0;
    m_saisoku_flag = saisoku_point;
    m_aite_hakka_rensa = 0;
    m_aite_hakka_nokori = 0;
    m_aite_hakka_zenkesi = 0;
    m_aite_hakka_kosuu = 0;
    m_nocc_aite_rensa_score = 0;
    m_aite_rensa_score = 0;
    m_aite_rensa_score_cc = 0;
    m_key_ee = 1;
    m_aite_hakkaji_score = 0;
    m_aite_hakka_jamako = 0;
    m_aite_hakka_honsen = 0;
    m_aite_puyo_uki = 0;
    m_aite_hakka_quick = 0;
    m_kougeki_on = 0;
    m_kougeki_edge = 0;
    m_kougeki_iryoku = 0;
    m_one_tanpatu = 1;
    m_score_max = 0;
    m_mmmax = -1;
    m_score_aa = -10;
    m_aa_max_score = 0;
    m_hakkatime = 0;
}

bool COMAI_HI::aite_attack_start(const PuyoColor ba3[6][kHeight], int zenkesi_aite, int scos, int hakata)
{
    int jamako_sabun;
    bool ret_keshi = false;

    int kosuu_mae = countNormalColor13(ba3);
    PuyoColor ba[6][kHeight] {};
    copyField(ba3, ba);

    TLRensaResult result = simulate(ba);
    int score = result.score;
    int quick = result.quick;
    m_aite_hakka_rensa = result.chains;
    m_aite_hakka_nokori = m_aite_hakka_rensa;
    m_hakkatime = hakata;
    if (m_aite_hakka_rensa > 0) {
        ret_keshi = true;
        m_aite_hakka_zenkesi = zenkesi_aite;
        jamako_sabun = m_aite_hakkaji_score / 70;
        m_aite_hakkaji_score = scos + score;
        m_aite_hakka_jamako = m_aite_hakkaji_score / 70 - jamako_sabun;
        m_aite_hakka_quick = quick;
        if (m_kougeki_on || m_kougeki_edge)
            m_aite_hakka_jamako -= m_kougeki_iryoku;
        if (m_aite_hakka_jamako < -11)
            m_aite_hakka_jamako += m_kougeki_iryoku;
        m_kougeki_on = 0;
        m_kougeki_iryoku = 0;
    }
    m_kougeki_edge = 0;

    int kosuu_ato = countNormalColor13(ba);

    m_aite_hakka_kosuu = kosuu_mae - kosuu_ato;
    if (m_aite_hakka_kosuu * 2 > kosuu_mae)
        m_aite_hakka_honsen = 1;

    return ret_keshi;
}

int COMAI_HI::aite_attack_nokori(const PuyoColor [6][kHeight], int hakata)
{
    m_aite_hakka_nokori = m_aite_hakka_rensa - (hakata - m_hakkatime + 30) / 40;
    return 0;
}

int COMAI_HI::aite_rensa_end()
{
    m_aite_hakka_rensa = 0;
    m_aite_hakka_zenkesi = 0;
    m_aite_hakka_kosuu = 0;
    m_aite_hakka_honsen = 0;
    m_saisoku_flag = saisoku_point;
    m_aite_puyo_uki = 0;
    m_aite_hakka_quick = 0;
    m_aite_hakka_jamako = 0;
    m_aite_hakka_nokori = 0;
    return 0;
}

int COMAI_HI::aite_hyouka(const PuyoColor ba3[6][kHeight], PuyoColor tsumo[])
{
    m_aite_rensa_score = 0;
    m_aite_rensa_score_cc = 0;
    m_nocc_aite_rensa_score = 0;

    PuyoColor nx1 = toValidPuyoColor(tsumo[0]);
    PuyoColor nx2 = toValidPuyoColor(tsumo[1]);
    PuyoColor nn1 = toValidPuyoColor(tsumo[2]);
    PuyoColor nn2 = toValidPuyoColor(tsumo[3]);
    int irokosuu = countNormalColor13(ba3);

    int unused_values[4];
    for (int aa = 0; aa < 22; ++aa) {
        if (tobashi_hantei_a(ba3, aa, nx1, nx2))
            continue;
        PuyoColor ba_a[6][kHeight] {};
        copyField(ba3, ba_a);
        setti_puyo(ba_a, aa, nx1, nx2, unused_values);
        TLRensaResult result_aa = simulate(ba_a);
        if (ba_a[2][11] != PuyoColor::EMPTY)
            continue;
        for (int bb = 0; bb < 22; ++bb) {
            if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                continue;
            PuyoColor ba_b[6][kHeight] {};
            copyField(ba_a, ba_b);
            setti_puyo(ba_b, bb, nn1, nn2, unused_values);
            TLRensaResult result_bb = simulate(ba_b);
            if (ba_b[2][11] != PuyoColor::EMPTY)
                continue;

            for (PuyoColor nk : NORMAL_PUYO_COLORS) {
                for (int dd = 0; dd < 6; ++dd) {
                    if (tobashi_hantei_a(ba_b, dd, nk, nk))
                        continue;
                    PuyoColor ba_d[6][kHeight] {};
                    copyField(ba_b, ba_d);
                    setti_puyo(ba_d, dd, nk, nk, unused_values);
                    TLRensaResult result_dd = simulate(ba_d);
                    if (ba_d[2][11] != PuyoColor::EMPTY)
                        continue;

                    int scores = result_aa.score;
                    int keshikos = result_aa.num_vanished;
                    if (scores < result_bb.score) {
                        scores = result_bb.score;
                        keshikos = result_bb.num_vanished;
                    }
                    if (scores < result_dd.score) {
                        scores = result_dd.score;
                        keshikos = result_dd.num_vanished;
                    }

                    if (m_aite_rensa_score < scores) {
                        m_aite_rensa_score = scores;
                    }
                    if (keshikos * 2 < irokosuu && m_nocc_aite_rensa_score < scores) {
                        m_nocc_aite_rensa_score = scores;
                        m_moni_kesiko[0] = result_aa.num_vanished;
                        m_moni_iroko[0] = result_aa.score;
                        m_moni_kesiko[1] = result_bb.num_vanished;
                        m_moni_iroko[1] = result_bb.score;
                        m_moni_kesiko[2] = result_dd.num_vanished;
                        m_moni_iroko[2] = result_dd.score;
                    }
                }
            }

            // Assume putting 2 same puyos vertically.
            for (int x = 0; x < 6; ++x) {
                for (PuyoColor color : NORMAL_PUYO_COLORS) {
                    PuyoColor ba[6][kHeight] {};
                    copyField(ba_b, ba);
                    for (int j = 0; j < 13; j++) {
                        if (ba[x][j] == PuyoColor::EMPTY) {
                            ba[x][j] = color;
                            ba[x][j + 1] = color;
                            break;
                        }
                    }

                    // Skip if the destination is unreachable.
                    if (ba[1][11] != PuyoColor::EMPTY && x == 0)
                        continue;
                    if (ba[2][11] != PuyoColor::EMPTY && x != 2)  // game over
                        continue;
                    if (ba[3][11] != PuyoColor::EMPTY && x > 3)
                        continue;
                    if (ba[4][11] != PuyoColor::EMPTY && x == 5)
                        continue;

                    TLRensaResult result = simulate(ba);
                    if (m_aite_rensa_score_cc < result.chains) {
                        m_aite_rensa_score_cc = result.chains;
                    }
                }
            }
        } // bb
    } // aa
    return 0;
}

int COMAI_HI::hyouka(const PuyoColor ba3[6][kHeight], PuyoColor tsumo[], int zenkesi_own, PuyoColor aite_ba[6][kHeight], int zenkesi_aite)
{
    int hym[22] {};
    int hyktmp;
    int keschk = 0;
    int maxch = 0, maxach = 0;
    int zenke[22] {};
    int setti_basyo[4];
    int myf_kosuu_kesi = 0, myf_kosuu_iro = 0;
    int kurai_large, kurai_middle, kurai_small;
    int aite_kosuu_iro = 0, kes2;
    int syuusoku = 0;
    int nidub_point_a[22] {};
    int maxscore = 0;
    int kuraichk = 0;
    int taiouchk = 0;
    int kurai_mini = 0;
    int score_tai = -10, tai_max_score = 0;
    int aite_ojama = 0;

    if (m_kougeki_on == 1) {
        m_kougeki_on = 0;
        m_kougeki_edge = 1;
    }

    m_myf_kosuu = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (ba3[i][j] != PuyoColor::EMPTY) {
                m_myf_kosuu++;
            }
            if (isNormalColor(ba3[i][j])) {
                myf_kosuu_iro++;
            }
        }
    }
    if (m_aite_hakka_rensa > 3)
        m_saisoku_flag = 0;

    bool shouldVanish = (m_myf_kosuu >= 55);
    bool shouldSaisoku = m_saisoku_flag && (m_myf_kosuu >= 38);

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (isNormalColor(aite_ba[i][j])) {
                aite_kosuu_iro++;
            }
            if (aite_ba[i][j] != PuyoColor::EMPTY) {
                aite_ojama++;
            }
        }
    }
    aite_ojama = aite_ojama - aite_kosuu_iro;
    if (aite_ojama > 5)
        m_one_tanpatu = 0;
    if ((myf_kosuu_iro >= aite_kosuu_iro * 2) && (m_cchai > 6) && (m_aite_hakka_rensa < 4)) {
        kes2 = 1;
    } else {
        kes2 = 0;
    }

    PuyoColor nx1 = toValidPuyoColor(tsumo[0]);
    PuyoColor nx2 = toValidPuyoColor(tsumo[1]);
    PuyoColor nn1 = toValidPuyoColor(tsumo[2]);
    PuyoColor nn2 = toValidPuyoColor(tsumo[3]);
    PuyoColor ba2[6][kHeight] {};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            ba2[i][j] = ba3[i][j];
        }
    }

    // Check 2-double
    int apos[22] {};
    if (m_hukks < 255) {
        for (int aa = 0; aa < 22; aa++) {
            if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                continue;
            PuyoColor ba_a[6][kHeight] {};
            copyField(ba2, ba_a);
            setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
            TLRensaResult result_aa = simulate(ba_a);
            if (result_aa.chains == 2 && result_aa.num_connections[1] >= 2)
                apos[aa] = 1;
            if (result_aa.num_vanished > 0 || ba_a[2][11] != PuyoColor::EMPTY)
                continue;

            for (int bb = 0; bb < 22; bb++) {
                if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                    continue;
                PuyoColor ba_ee[6][kHeight];
                copyField(ba_a, ba_ee);
                setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                TLRensaResult result_bb = simulate(ba_ee);
                if (result_bb.chains == 2 && result_bb.num_connections[1] >= 2)
                    apos[aa] = 1;
            }
        }
    }

    for (int aa = 0; aa < 22; aa++) {
        hym[aa] = -10000;
        zenke[aa] = 0;
    }

    kurai_large = 0;
    kurai_middle = 0;
    kurai_small = 0;
    if (m_aite_hakka_nokori < 3) {
        if ((((m_aite_hakka_rensa > 3) || (m_aite_hakka_kosuu > 15)) && (m_aite_hakka_jamako > 4))
            || ((m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako > 35))
            || ((m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako > 35))) {
            kurai_large = 1;
        } else if ((m_aite_hakka_kosuu > 12) && (m_aite_hakka_jamako > 4)) {
            kurai_middle = 1;
        } else if ((m_aite_hakka_kosuu > 8) && (m_aite_hakka_jamako > 4)) {
            kurai_small = 1;
        } else if ((m_aite_hakka_jamako > config.y_t)) {
            kurai_mini = 1;
        }
    }
    if (m_aite_hakka_nokori < 5) {
        if ((m_aite_hakka_rensa > 3) || (m_aite_hakka_kosuu > 15) || (m_aite_hakka_zenkesi == 1)) {
            syuusoku = 1;
        } else if ((m_aite_hakka_kosuu > 12)) {
            syuusoku = 1;
        }
    }

    bool shouldClearWariko = false;
    if ((((m_aite_hakka_rensa > 3) || (m_aite_hakka_kosuu > 15)) && (m_aite_hakka_jamako > 4))
        || ((m_aite_hakka_zenkesi == 1) && (zenkesi_own != 1))) {
        shouldClearWariko = true;
    } else if ((m_aite_hakka_kosuu > 12) && (m_aite_hakka_jamako > 4)) {
        shouldClearWariko = true;
    } else if ((m_aite_hakka_kosuu > 8) && (m_aite_hakka_jamako > 4)) {
        shouldClearWariko = true;
    } else if ((m_aite_hakka_jamako > 3)) {
        shouldClearWariko = true;
    }

    // Check Zenkeshi
    int zenchk = 0;
    int zenchain = 0;
    if (m_aite_hakka_rensa < 5) {
        if (m_myf_kosuu < 15) {
            for (int aa = 0; aa < 22; aa++) {
                PuyoColor ba_a[6][kHeight] {};
                copyField(ba2, ba_a);
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                int chig_aa = isChigiri(setti_basyo);
                TLRensaResult result_aa = simulate(ba_a);
                int score = result_aa.score;
                if (isTLFieldEmpty(ba_a)) {
                    int zenkes = score + 2100 - chig_aa * 3;
                    if (zenchain <= zenkes) {
                        zenchain = zenkes;
                        zenchk = aa;
                    }
                }

                if (m_myf_kosuu < 13) {
                    for (int bb = 0; bb < 22; bb++) {
                        PuyoColor ba_ee[6][kHeight];
                        copyField(ba_a, ba_ee);
                        setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                        int chig_bb = isChigiri(setti_basyo);
                        TLRensaResult result_bb = simulate(ba_ee);
                        int score = result_bb.score;
                        if (isTLFieldEmpty(ba_ee)) {
                            int zenkes = score + 2100 - (chig_aa + chig_bb) * 3;
                            if (zenchain <= zenkes) {
                                zenchain = zenkes;
                                zenchk = aa;
                            }
                        }
                    }
                } // p<6
            }
        } // p<7
    }

    if (zenchain > 2100)
        zenke[zenchk] += 120000;

    for (int aa = 0; aa < 22; aa++) {
        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;

        PuyoColor ba[6][kHeight] {};
        copyField(ba2, ba);
        setti_puyo(ba, aa, nx1, nx2, setti_basyo);
        TLRensaResult result_aa = simulate(ba);
        int chain = result_aa.chains;
        int score = result_aa.score;

        // つぶし
        // 次の条件で潰しを打つ。
        // 1. 相手の発火が4連鎖未満
        // 2. こちらの連鎖が2連鎖
        // 3. こちらの連鎖が相手より260点超大きい
        // 4. こちらの連鎖のスコアが1600を上回る
        if ((config.e_t) && (m_aite_hakka_rensa < 4)) {
            if ((chain == 2) && (score > m_aite_rensa_score + 260) && (score > 1600)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }

        // つぶし
        // 1. 現在のターン数が tubushiturn を超え
        // 2. 相手の発火が4連鎖未満かつ、相手の色個数が12を超え
        // 3. 次の条件のうちいずれかを満たす。
        //  3a. 自分の連鎖が2連鎖、相手の発火より260点を超えて上回り、スコアが690を超えている。
        //  3b. 自分の連鎖が3連鎖、相手の連鎖が3連鎖未満 (?)
        if ((m_hukks > tubushiturn) && (m_aite_hakka_rensa < 4) && (aite_kosuu_iro > 12)) {
            if ((chain == 2) && (score > m_aite_rensa_score + 260) && (score > 690)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
            if (chain == 3 && m_aite_rensa_score_cc < 3) {
                hym[aa] += 72000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }
        if ((maxch <= chain) && (maxscore < score)) {
            maxch = chain;
            maxach = aa;
            maxscore = score;
        }
        myf_kosuu_kesi = 0;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 13; j++) {
                if (isNormalColor(ba[i][j])) {
                    myf_kosuu_kesi++;
                }
            }
        }
        if ((chain == 1) && (score > 690) && (m_aite_hakka_rensa == 0)) {
            hym[aa] += 74000;
            keschk = 1;
        }
        // さいそく
        if (shouldSaisoku) {
            if ((myf_kosuu_iro - myf_kosuu_kesi + 10 < myf_kosuu_kesi) && (score > m_nocc_aite_rensa_score + 130 * chain)
                && (chain > 1) && (m_aite_hakka_rensa == 0)) {
                hym[aa] += 73000;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }
        // kurai 対応
        if (config.u_t == 0) {
            if (((score > m_aite_hakka_jamako * 70 - 280) && (score < m_aite_hakka_jamako * 70 + 1400))
                && ((kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1)) && (m_aite_hakka_honsen == 0)
                && (chain > 1)) {
                hym[aa] += 120000;
                keschk = 1;
            }
        }
        if (zenkesi_own == 1) { // aaaa0909
            if ((chain == 1) && (m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako < 36)) { // add121014
                hym[aa] += 74000;
            }
            if ((chain > 0) && (zenkesi_aite != 1)) {
                hym[aa] += 74000;
            }
        }
    } // aa
    if (kes2 == 1) {
        if ((maxch > m_cchai) && (m_cchai > 1)) {
            hym[maxach] += 150000;
            keschk = 1;
        } else if ((maxch > m_cchai - 1) && (m_cchai - 1 > 1)) {
            hym[maxach] += 148000;
            keschk = 1;
        } else if ((maxch > m_cchai - 2) && (m_cchai - 2 > 1)) {
            hym[maxach] += 146000;
            keschk = 1;
        }
    }
    if (m_aite_hakka_honsen > 0 || keschk)
        shouldClearWariko = false;

    if (((kurai_mini == 1) || (kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1))
        && (m_aite_hakka_honsen == 0) && (keschk == 0)) {
        taiouchk = 1;
    }
    if (((kurai_mini == 1) && (keschk == 0) && (m_myf_kosuu > 64)) || ((kurai_large == 1) && (keschk == 0))
        || ((kurai_middle == 1) && (keschk == 0)) || ((kurai_small == 1) && (keschk == 0))
        || ((m_aite_hakka_honsen == 1) && (m_aite_hakka_nokori < 3))) {
        kuraichk = 1;
    }

    if ((m_aite_hakka_honsen == 0) && config.q_t) {
        if (m_myf_kosuu > 23) {
            for (int aa = 0; aa < 22; aa++) {
                if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                    continue;
                PuyoColor ba_a[6][kHeight] {};
                copyField(ba2, ba_a);
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                TLRensaResult result_aa = simulate(ba_a);
                if (result_aa.num_vanished != 0)
                    continue;
                for (int cplace = 0; cplace < 6; cplace++) {
                    if (ba_a[0][11] != PuyoColor::EMPTY) {
                        if (cplace == 0)
                            continue;
                    }
                    if (ba_a[1][11] != PuyoColor::EMPTY) {
                        if ((cplace == 0) || (cplace == 1))
                            continue;
                    }
                    if (ba_a[3][11] != PuyoColor::EMPTY) {
                        if ((cplace == 3) || (cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[4][11] != PuyoColor::EMPTY) {
                        if ((cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[5][11] != PuyoColor::EMPTY) {
                        if (cplace == 5)
                            continue;
                    }
                    for (int cyy = 0; cyy < 2; cyy++) {
                        for (PuyoColor ccolor : NORMAL_PUYO_COLORS) {
                            PuyoColor ba[6][kHeight] {};
                            copyField(ba_a, ba);
                            int coita = 0;
                            for (int j = 0; j < (12 - cyy); j++) {
                                if (ba[cplace][j + cyy] == PuyoColor::EMPTY) {
                                    ba[cplace][j + cyy] = ccolor;
                                    coita = 1;
                                    setti_basyo[0] = cplace;
                                    setti_basyo[1] = j + cyy;
                                    setti_basyo[2] = -1;
                                    setti_basyo[3] = -1;
                                    break;
                                }
                            }
                            if (coita == 0)
                                continue;
                            TLRensaResult result = simulate(ba);
                            int chain = result.chains;
                            if (config.is_2dub_cpu) {
                                if ((m_myf_kosuu < 46) && (chain == 2) && (result.num_connections[1] > 1))
                                    nidub_point_a[aa] = 3000;
                            } else {
                                if ((m_myf_kosuu < 46) && (chain == 2) && (result.num_connections[1] > 1))
                                    nidub_point_a[aa] = 1200;
                            }
                        }
                    } // cyy
                } // cc
            } // aa
        }

    } // m_aite_hakka_honsen==0

    if (config.a_t == 0)
        shouldClearWariko = taiouchk;

    m_cchai = 0;

    for (int aa = 0; aa < 22; aa++) {
        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;
        PuyoColor ba_a[6][kHeight] {};
        copyField(ba2, ba_a);
        setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
        TLRensaResult result_aa = simulate(ba_a);
        int keshiko_aa = result_aa.num_vanished;
        int score_tmp = result_aa.score;
        int score_aonly = score_tmp; // only
        int score_tmp2 = score_tmp;
        if ((ba_a[2][11] == PuyoColor::EMPTY) && (score_tmp > m_score_aa)) {
            m_score_aa = score_tmp;
            m_aa_max_score = aa;
            m_tesuu_mon = 1;
        }
        if ((ba_a[2][11] == PuyoColor::EMPTY) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
            && (m_aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > m_aite_hakka_jamako * 70)) {
            if ((myf_kosuu_iro - 1 > keshiko_aa * 2) || (config.i_t)) {
                score_tai = score_tmp2;
                tai_max_score = aa;
            }
        }
        if ((myf_kosuu_iro - keshiko_aa + 8) < m_cchai * 4)
            continue;
        if (ba_a[2][11] != PuyoColor::EMPTY) {
            // In case the 1st control leads to die, do not score following controls.
            for (int bb = 0; bb < 22; bb++) {
                for (int dd = 0; dd < 221; dd++) {
                    for (int ee = 0; ee < EE_SIZE; ee++) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
            }
            continue;
        } // 110604

        for (int bb = 0; bb < 22; bb++) {
            if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                continue;
            PuyoColor ba_ee[6][kHeight];
            copyField(ba_a, ba_ee);
            setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
            TLRensaResult result_bb = simulate(ba_ee);
            int keshiko_bb = result_bb.num_vanished;
            int score_tmp = result_bb.score;
            int score_tmp2 = score_tmp;
            if ((kuraichk == 1) && (m_aite_hakka_nokori < 2) && (score_aonly > 0))
                score_tmp = 0; // only
            if ((kuraichk == 1) && ((m_aite_hakka_nokori < 1)
                                    || ((m_aite_hakka_nokori < 2) && ((m_aite_puyo_uki == 0) && (m_aite_hakka_quick == 1)))))
                score_tmp = 0; // 0225
            if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp > m_score_aa)) {
                m_score_aa = score_tmp;
                m_aa_max_score = aa;
                m_tesuu_mon = 2;
            }
            if (shouldClearWariko) {
                if ((m_aite_hakka_nokori < 1)
                    || (m_aite_hakka_nokori < 2 && score_aonly > 0)
                    || (m_aite_hakka_nokori < 2 && m_aite_puyo_uki == 0 && m_aite_hakka_quick == 1))
                    score_tmp2 = 0; // 0225
            }
            if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                && (m_aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > m_aite_hakka_jamako * 70)) {
                if ((myf_kosuu_iro + 1 > (keshiko_aa + keshiko_bb) * 2) || (config.i_t)) {
                    score_tai = score_tmp2;
                    tai_max_score = aa;
                }
            }
            if ((myf_kosuu_iro - keshiko_aa - keshiko_bb + 8) < m_cchai * 4)
                continue;
            if (ba_ee[2][11] != PuyoColor::EMPTY) {
                for (int ee = 0; ee < EE_SIZE; ee++) {
                    for (int dd = 0; dd < 221; dd++) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
                continue;
            } // 110604
        } // bb
    } // aa

    for (int aa = 0; aa < 22; aa++) {
        for (int bb = 0; bb < 22; bb++) {
            for (int dd = 0; dd < 221; dd++) {
                // TODO(peria): Skip some cases if next2 tsumo is known.
                if (!isMatchIndexAndColors(dd, &tsumo[4]))
                    continue;
                for (int ee = 0; ee < EE_SIZE; ee++) {
                    if ((m_key_ee == 0) && (ee > 0))
                        break; // t2
                    hyktmp = g_chainhyk[aa][bb][dd][ee] * 1000 + g_poihyo[aa][bb][dd][ee]
                             + (((m_myf_kosuu > 40) || syuusoku) * (ee == 0) * 300) + g_score_hukasa[aa][bb][dd];
                    if (hym[aa] < hyktmp) {
                        hym[aa] = hyktmp;
                        m_max_ee = ee;
                    }
                }
            }
        }
    }
    if (((m_myf_kosuu > 50) || (syuusoku == 1)) && (m_max_ee == 0))
        m_key_ee = 0;
    if ((m_myf_kosuu < 44) && (syuusoku == 0))
        m_key_ee = 1;
    for (int aa = 0; aa < 22; aa++) {
        hym[aa] += zenke[aa];
        hym[aa] += nidub_point_a[aa];
    }
    m_kuraichk_mon = kuraichk;
    m_score_mon = -1;
    if ((shouldVanish && (m_score_max < m_score_aa)) || ((kuraichk == 1) && (m_score_aa > 530))) {
        hym[m_aa_max_score] += 100000;
        m_score_mon = m_score_aa;
    } else if (shouldVanish && (m_mmmax != -1)) {
        hym[m_mmmax] += 60000;
        m_score_mon = m_score_aa;
    }
    if (config.u_t == 1) {
        if (shouldClearWariko && (((score_tai > 270) && ((m_myf_kosuu > 36) || (ba2[2][10] != PuyoColor::EMPTY)
                                                         || (m_aite_hakka_zenkesi == 1))) || (score_tai > 840))) {
            hym[tai_max_score] += 140000;
            m_score_mon = score_tai;
        }
    }

    // 低い場所
    const int yokoyose = 2;
    for (int i = 0; i < 6; i++) {
        int n = 0;
        for (int j = 0; j < 13; j++) {
            if (ba2[i][j] == PuyoColor::EMPTY)
                n++;
        }
        hym[i] += n * 2;
        hym[i + 6] += n * 2;
        if (i != 0)
            hym[i + 11] += n * 1;
        if (i != 5)
            hym[i + 12] += n * 1;
        if (i != 0)
            hym[i + 16] += n * 1;
        if (i != 5)
            hym[i + 17] += n * 1;
        if ((i == 0) || (i == 5)) {
            hym[i] += yokoyose * 4;
            hym[i + 6] += yokoyose * 4;
            if (i != 0)
                hym[i + 11] += yokoyose * 2;
            if (i != 5)
                hym[i + 12] += yokoyose * 2;
            if (i != 0)
                hym[i + 16] += yokoyose * 2;
            if (i != 5)
                hym[i + 17] += yokoyose * 2;
        }
        if ((i == 1) || (i == 4)) {
            hym[i] += yokoyose * 2;
            hym[i + 6] += yokoyose * 2;
            if (i != 0)
                hym[i + 11] += yokoyose;
            if (i != 5)
                hym[i + 12] += yokoyose;
            if (i != 0)
                hym[i + 16] += yokoyose;
            if (i != 5)
                hym[i + 17] += yokoyose;
        }
    }

    if ((zenkesi_own == 1) && (myf_kosuu_iro == 0)) {
        hym[5] += 5000;
        hym[11] += 5000;
        hym[16] += 5000;
        hym[21] += 5000;
    }

    // chigiri
    if (((zenkesi_own == 1) && (zenkesi_aite != 1)) || ((m_aite_hakka_honsen == 0) && (config.w_t)) || (zenkesi_aite == 1)) {
        for (int aa = 0; aa < 22; aa++) {
            PuyoColor ba[6][kHeight] {};
            copyField(ba2, ba);
            if (aa < 6) {
                for (int j = 0; j < 13; j++) {
                    if (ba[aa][j] == PuyoColor::EMPTY) {
                        ba[aa][j] = nx1;
                        ba[aa][j + 1] = nx2;
                        break;
                    }
                }
            } else if (aa < 12) {
                for (int j = 0; j < 13; j++) {
                    if (ba[aa - 6][j] == PuyoColor::EMPTY) {
                        ba[aa - 6][j] = nx2;
                        ba[aa - 6][j + 1] = nx1;
                        break;
                    }
                }
            } else if (aa < 17) {
                for (int j = 0; j < 13; j++) {
                    if (ba[aa - 11][j] == PuyoColor::EMPTY) {
                        ba[aa - 11][j] = nx1;
                        break;
                    }
                }
                for (int j = 0; j < 13; j++) {
                    if (ba[aa - 12][j] == PuyoColor::EMPTY) {
                        ba[aa - 12][j] = nx2;
                        break;
                    }
                }
            } else if (aa < 22) {
                for (int j = 0; j < 13; j++) {
                    if (ba[aa - 17][j] == PuyoColor::EMPTY) {
                        ba[aa - 17][j] = nx1;
                        break;
                    }
                }
                for (int j = 0; j < 13; j++) {
                    if (ba[aa - 16][j] == PuyoColor::EMPTY) {
                        ba[aa - 16][j] = nx2;
                        break;
                    }
                }
            }

            int num = 0;
            Check point[6][12] {};
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 12; j++) {
                    if (ba[i][j] == PuyoColor::EMPTY)
                        break;
                    if (point[i][j] != Check::Checked && isNormalColor(ba[i][j])) {
                        saiki(ba, point, i, j, &num, ba[i][j]);
                        if (num != 0) {
                            if (num == 2)
                                hym[aa] += 30 * config.renketu_bairitu;
                            if (num == 3)
                                hym[aa] += 120 * config.renketu_bairitu;
                            if ((zenkesi_aite == 1) && (num == 2))
                                hym[aa] += 120;
                            if ((zenkesi_aite == 1) && (num == 3))
                                hym[aa] += 480;
                            num = 0;
                        }
                    }
                }
            }
        }
    }

    for (int aa = 0; aa < 22; aa++) {
        if ((m_myf_kosuu < 52) && (myf_kosuu_iro > 23) && (m_aite_hakka_rensa < 4) && (m_aite_hakka_honsen == 0))
            hym[aa] += apos[aa] * 2000;
        if (config.is_2dub_cpu) {
            if ((m_myf_kosuu < 52) && (m_aite_hakka_rensa < 4) && (m_aite_hakka_honsen == 0))
                hym[aa] += apos[aa] * 2000;
        }
    }

    static const int bias[] = {4,4,4,4,4,4, 3,3,3,3,3,3, 2,2,2,2,2, 1,1,1,1,1};
    for (int aa = 0; aa < 22; aa++) {
        m_para[aa] = hym[aa] * 10 + bias[aa];
    }
    return 0;
}

int COMAI_HI::tobashi_hantei_a(const PuyoColor ba[][kHeight], int aa, PuyoColor nx1, PuyoColor nx2)
{
    if (nx1 == nx2) {
        if ((aa > 5 && aa < 12) || (aa > 13 && aa < 19))
            return 1;
    }
    return tobashi_hantei_b(ba, aa);
}

int COMAI_HI::tobashi_hantei_b(const PuyoColor ba2[][kHeight], int aa)
{
    if (ba2[0][11] != PuyoColor::EMPTY) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
    }
    if (ba2[1][11] != PuyoColor::EMPTY) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
        if ((aa == 1) || (aa == 7) || (aa == 13) || (aa == 18))
            return 1;
    }
    if (ba2[3][11] != PuyoColor::EMPTY) {
        if ((aa == 3) || (aa == 9) || (aa == 14) || (aa == 19))
            return 1;
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[4][11] != PuyoColor::EMPTY) {
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[5][11] != PuyoColor::EMPTY) {
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    return 0;
}

int COMAI_HI::pre_hyouka(const PuyoColor ba3[6][kHeight], PuyoColor tsumo[], int zenkesi_own, PuyoColor aite_ba[6][kHeight],
                         int zenkesi_aite, int fast)
{
    PuyoColor ba_a[6][kHeight] {};
    PuyoColor ba2[6][kHeight] {};
    int hym[22];
    int keschk = 0;
    int maxch = 0, maxach;
    int teimen[6];
    int zenkes[22][22][22] {};
    int zenke[22] = { 0 };
    int setti_basyo[4];
    int myf_kosuu_kesi = 0, myf_kosuu_iro = 0;

    PuyoColor eecol;
    PuyoColor ba_ee[6][kHeight];
    int keshiko_dd;

    int maxscore = 0;
    int score_mm = 0;
    int kuraichk = 0;
    int taiouchk = 0;
    int kurai_mini = 0;
    int score_tai = -10;
    int aveteimen = 0;
    int hakkatakasa = 0;
    int chig_aa, chig_bb, chig_dd;
    int tanpatu_on = 0;

    m_hukks++;

    m_score_max = 0;
    m_mmmax = -1;
    m_score_aa = -10;
    m_aa_max_score = 0;

    if (m_kougeki_on == 1) {
        m_kougeki_on = 0;
        m_kougeki_edge = 1;
    }

    m_myf_kosuu = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (ba3[i][j] != PuyoColor::EMPTY) {
                m_myf_kosuu++;
            }
            if (isNormalColor(ba3[i][j])) {
                myf_kosuu_iro++;
            }
        }
    }

    if (m_aite_hakka_rensa > 3)
        m_saisoku_flag = 0;
    int saisoku = (m_saisoku_flag) && (m_myf_kosuu / 38);

    int aite_kosuu_iro = 0;
    int aite_ojama = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (isNormalColor(aite_ba[i][j])) {
                aite_kosuu_iro++;
            }
            if (aite_ba[i][j] != PuyoColor::EMPTY) {
                aite_ojama++;
            }
        }
    }
    aite_ojama -= aite_kosuu_iro;
    if (aite_ojama > 5)
        m_one_tanpatu = 0;

    int kes2 = 0;
    if ((myf_kosuu_iro >= aite_kosuu_iro * 2) && (m_cchai > 6) && (m_aite_hakka_rensa < 4))
        kes2 = 1;

    PuyoColor nx1 = toValidPuyoColor(tsumo[0]);
    PuyoColor nx2 = toValidPuyoColor(tsumo[1]);
    PuyoColor nn1 = toValidPuyoColor(tsumo[2]);
    PuyoColor nn2 = toValidPuyoColor(tsumo[3]);
    PuyoColor nk1 = PuyoColor::RED;
    PuyoColor nk2 = PuyoColor::RED;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            ba2[i][j] = ba3[i][j];
        }
    }

    for (int dd = 0; dd < 221; dd++) {
        for (int aa = 0; aa < 22; aa++) {
            for (int bb = 0; bb < 22; bb++) {
                g_score_hukasa[aa][bb][dd] = 3000;
                for (int ee = 0; ee < EE_SIZE; ee++) {
                    g_chainhyk[aa][bb][dd][ee] = 0;
                    g_poihyo[aa][bb][dd][ee] = -2000;
                }
            }
        }
    }

    for (int aa = 0; aa < 22; aa++) {
        hym[aa] = -10000;
        zenke[aa] = 0;
    }

    int kurai_large = 0;
    int kurai_middle = 0;
    int kurai_small = 0;
    if (m_aite_hakka_nokori < 3) {
        if ((((m_aite_hakka_rensa > 3) || (m_aite_hakka_kosuu > 15)) && (m_aite_hakka_jamako > 4))
            || ((m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako > 35))
            || ((m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako > 35))) {
            kurai_large = 1;
        } else if ((m_aite_hakka_kosuu > 12) && (m_aite_hakka_jamako > 4)) {
            kurai_middle = 1;
        } else if ((m_aite_hakka_kosuu > 8) && (m_aite_hakka_jamako > 4)) {
            kurai_small = 1;
        } else if ((m_aite_hakka_jamako > config.y_t)) {
            kurai_mini = 1;
        }
    }

    int wariko_taiou = 0;
    if ((((m_aite_hakka_rensa > 3) || (m_aite_hakka_kosuu > 15)) && (m_aite_hakka_jamako > 4))
        || ((m_aite_hakka_zenkesi == 1) && (zenkesi_own != 1))) {
        wariko_taiou = 1;
    } else if ((m_aite_hakka_kosuu > 12) && (m_aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((m_aite_hakka_kosuu > 8) && (m_aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((m_aite_hakka_jamako > 3)) {
        wariko_taiou = 1;
    }

    if (m_aite_hakka_rensa < 5) {
        if (m_myf_kosuu < 15) {
            for (int aa = 0; aa < 22; aa++) {
                copyField(ba2, ba_a);
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                chig_aa = isChigiri(setti_basyo);
                TLRensaResult result_aa = simulate(ba_a);
                int score = result_aa.score;
                if (isTLFieldEmpty(ba_a)) {
                    for (int bb = 0; bb < 22; bb++) {
                        for (int dd = 0; dd < 22; dd++) {
                            zenkes[aa][bb][dd] += score + 2100 - chig_aa * 3;
                        }
                    }
                }

                if (m_myf_kosuu < 13) {
                    for (int bb = 0; bb < 22; bb++) {
                        copyField(ba_a, ba_ee);
                        setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                        chig_bb = isChigiri(setti_basyo);
                        TLRensaResult result_bb = simulate(ba_ee);
                        int score = result_bb.score;
                        if (isTLFieldEmpty(ba_ee)) {
                            for (int dd = 0; dd < 22; dd++) {
                                zenkes[aa][bb][dd] += score + 2100 - (chig_aa + chig_bb) * 3;
                            }
                        }
                    }
                } // p<6
            }
        } // p<7
    }

    int zenchk = 0;
    int zenchain = 0;
    for (int aa = 0; aa < 22; aa++) {
        for (int bb = 0; bb < 22; bb++) {
            for (int dd = 0; dd < 22; dd++) {
                if (zenchain <= zenkes[aa][bb][dd]) {
                    zenchain = zenkes[aa][bb][dd];
                    zenchk = aa;
                }
            }
        }
    }
    if (zenchain > 2100)
        zenke[zenchk] += 120000;

    int num = 0;

    for (int aa = 0; aa < 22; aa++) {
        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;
        PuyoColor ba[6][kHeight] {};
        copyField(ba2, ba);
        setti_puyo(ba, aa, nx1, nx2, setti_basyo);
        TLRensaResult result = simulate(ba);
        int chain = result.chains;
        int score = result.score;

        // つぶし
        if ((config.e_t) && (m_aite_hakka_rensa < 4)) {
            if ((chain == 2) && (score > m_aite_rensa_score + 260) && (score > 1600)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }

        if ((m_hukks > tubushiturn) && (m_aite_hakka_rensa < 4) && (aite_kosuu_iro > 12)) {
            if ((chain == 2) && (score > m_aite_rensa_score + 260) && (score > 690)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
            if (((chain == 3) || (chain == 4)) && (m_aite_rensa_score_cc < 3)) {
                hym[aa] += 72000 + score;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }
        if ((maxch <= chain) && (maxscore < score)) {
            maxch = chain;
            maxach = aa;
            maxscore = score;
        }
        myf_kosuu_kesi = 0;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 13; j++) {
                if (isNormalColor(ba[i][j])) {
                    myf_kosuu_kesi++;
                }
            }
        }
        if ((chain == 1) && (score > 690) && (m_aite_hakka_rensa == 0)) {
            hym[aa] += 74000;
            keschk = 1;
        }
        // さいそく
        if (saisoku == 1) {
            if ((myf_kosuu_iro - myf_kosuu_kesi + 10 < myf_kosuu_kesi) && (score > m_nocc_aite_rensa_score + 130 * chain)
                && (chain > 1) && (m_aite_hakka_rensa == 0)) {
                hym[aa] += 73000;
                keschk = 1;
                m_kougeki_iryoku = score / 70;
                m_kougeki_on = 1;
            }
        }
        // kurai 対応
        if (config.u_t == 0) {
            if (((score > m_aite_hakka_jamako * 70 - 280) && (score < m_aite_hakka_jamako * 70 + 1400))
                && ((kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1)) && (m_aite_hakka_honsen == 0)
                && (chain > 1)) {
                hym[aa] += 120000;
                keschk = 1;
            }
        }
        if (zenkesi_own == 1) { // aaaa0909
            if ((chain == 1) && (m_aite_hakka_zenkesi == 1) && (m_aite_hakka_jamako < 36)) { // add121014
                hym[aa] += 74000;
            }
            if ((chain > 0) && (zenkesi_aite != 1)) {
                hym[aa] += 74000;
            }
        }
    } // aa

    if (kes2 == 1) {
        if ((maxch > m_cchai) && (m_cchai > 1)) {
            hym[maxach] += 150000;
            keschk = 1;
        } else if ((maxch > m_cchai - 1) && (m_cchai - 1 > 1)) {
            hym[maxach] += 148000;
            keschk = 1;
        } else if ((maxch > m_cchai - 2) && (m_cchai - 2 > 1)) {
            hym[maxach] += 146000;
            keschk = 1;
        }
    }
    if ((wariko_taiou == 1) && (m_aite_hakka_honsen == 0) && (keschk == 0)) {
        wariko_taiou = 1;
    } else {
        wariko_taiou = 0;
    }

    if (((kurai_mini == 1) || (kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1))
        && (m_aite_hakka_honsen == 0) && (keschk == 0)) {
        taiouchk = 1;
    }
    if (((kurai_mini == 1) && (keschk == 0) && (m_myf_kosuu > 64)) || ((kurai_large == 1) && (keschk == 0))
        || ((kurai_middle == 1) && (keschk == 0)) || ((kurai_small == 1) && (keschk == 0))
        || ((m_aite_hakka_honsen == 1) && (m_aite_hakka_nokori < 3))) {
        kuraichk = 1;
    }

    num = 0;

    if (config.a_t == 0)
        wariko_taiou = taiouchk;

    m_cchai = 0;
    m_tesuu_mon = 0;
    chig_aa = 0;
    chig_bb = 0;
    chig_dd = 0;

    for (int dd = 0; dd < 221; dd++) {
        if (dd < 220 && fast)
            continue;

        int color = dd / 22;
        int hand = dd % 22;
        if (dd < 22 * 10) {
            nk1 = kTsumoPatterns[color][0];
            nk2 = kTsumoPatterns[color][1];
        } else {  // dd == 220
            nk1 = PuyoColor::EMPTY;
            nk2 = PuyoColor::EMPTY;
        }
        if (nk1 == nk2 && 5 < hand && hand < 12)
            continue;

        for (int aa = 0; aa < 22; aa++) {
            if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                continue;
            copyField(ba2, ba_a);
            setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
            int chig_aa = isChigiri(setti_basyo);
            TLRensaResult result_aa = simulate(ba_a);
            int keshiko_aa = result_aa.num_vanished;
            int score_tmp = result_aa.score;
            int score_aonly = score_tmp; // only
            int score_tmp2 = score_tmp;
            if ((ba_a[2][11] == PuyoColor::EMPTY) && (score_tmp > m_score_aa)) {
                m_score_aa = score_tmp;
                m_aa_max_score = aa;
                m_tesuu_mon = 1;
            }
            if ((ba_a[2][11] == PuyoColor::EMPTY) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                && (m_aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > m_aite_hakka_jamako * 70)) {
                if ((myf_kosuu_iro - 1 > keshiko_aa * 2) || (config.i_t)) {
                    score_tai = score_tmp2;
                }
            }
            if ((myf_kosuu_iro - keshiko_aa + 8) < m_cchai * 4)
                continue;
            if (ba_a[2][11] != PuyoColor::EMPTY) {
                for (int bb = 0; bb < 22; bb++) {
                    for (int ee = 0; ee < EE_SIZE; ee++) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
                continue;
            } // 110604

            for (int bb = 0; bb < 22; bb++) {
                if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                    continue;
                copyField(ba_a, ba_ee);
                setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                int chig_bb = isChigiri(setti_basyo);
                TLRensaResult result_bb = simulate(ba_ee);
                int keshiko_bb = result_bb.num_vanished;
                int score_tmp = result_bb.score;
                int score_bonly = score_tmp; // only
                int score_tmp2 = score_tmp;
                if ((kuraichk == 1) && (m_aite_hakka_nokori < 2) && (score_aonly > 0))
                    score_tmp = 0; // only
                if ((kuraichk == 1)
                    && ((m_aite_hakka_nokori < 1)
                        || ((m_aite_hakka_nokori < 2) && ((m_aite_puyo_uki == 0) && (m_aite_hakka_quick == 1)))))
                    score_tmp = 0; // 0225
                if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp > m_score_aa)) {
                    m_score_aa = score_tmp;
                    m_aa_max_score = aa;
                    m_tesuu_mon = 2;
                }
                if ((wariko_taiou == 1) && (m_aite_hakka_nokori < 2) && (score_aonly > 0))
                    score_tmp2 = 0; // only
                if ((wariko_taiou == 1)
                    && ((m_aite_hakka_nokori < 1)
                        || ((m_aite_hakka_nokori < 2) && ((m_aite_puyo_uki == 0) && (m_aite_hakka_quick == 1)))))
                    score_tmp2 = 0; // 0225
                if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                    && (m_aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > m_aite_hakka_jamako * 70)) {
                    if ((myf_kosuu_iro + 1 > (keshiko_aa + keshiko_bb) * 2) || (config.i_t)) {
                        score_tai = score_tmp2;
                    }
                }
                if ((myf_kosuu_iro - keshiko_aa - keshiko_bb + 8) < m_cchai * 4)
                    continue;
                if (ba_ee[2][11] != PuyoColor::EMPTY) {
                    for (int ee = 0; ee < EE_SIZE; ee++) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -3000;
                    }
                    continue;
                } // 110604

                // NOTE: In case dd==220, we assume the 3rd Tsumo is (EMPTY,EMPTY).
                if (dd < 220) {
                    if (tobashi_hantei_b(ba_ee, dd % 22))
                        continue;
                    setti_puyo(ba_ee, dd % 22, nk1, nk2, setti_basyo);
                    chig_dd = isChigiri(setti_basyo);
                    keshiko_dd = chousei_syoukyo_sc(ba_ee, setti_basyo, &score_tmp);
                    score_tmp2 = score_tmp;
                    if ((kuraichk == 1) && ((score_aonly > 0) || (score_bonly > 0)))
                        score_tmp = 0; // only
                    if ((kuraichk == 1) && (m_aite_hakka_nokori < 3))
                        score_tmp = 0;

                    if (m_myf_kosuu < 56) {
                        if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp > m_score_aa)) {
                            m_score_aa = score_tmp;
                            m_aa_max_score = aa;
                            m_tesuu_mon = 3;
                        }
                    } else if (m_myf_kosuu < 62) {
                        if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp * 6 / 7 > m_score_aa)) {
                            m_score_aa = score_tmp * 6 / 7;
                            m_aa_max_score = aa;
                            m_tesuu_mon = 3;
                        }
                    } else if (m_myf_kosuu < 64) {
                        if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp * 3 / 4 > m_score_aa)) {
                            m_score_aa = score_tmp * 3 / 4;
                            m_aa_max_score = aa;
                            m_tesuu_mon = 3;
                        }
                    } else {
                        if ((ba_ee[2][11] == PuyoColor::EMPTY) && (score_tmp * 1 / 2 > m_score_aa)) {
                            m_score_aa = score_tmp * 1 / 2;
                            m_aa_max_score = aa;
                            m_tesuu_mon = 3;
                        }
                    }
                    if ((myf_kosuu_iro - keshiko_aa - keshiko_bb - keshiko_dd + 8) < m_cchai * 4)
                        continue;
                    if (ba_ee[2][11] != PuyoColor::EMPTY) {
                        for (int ee = 0; ee < EE_SIZE; ee++) {
                            g_chainhyk[aa][bb][dd][ee] = 0;
                            g_poihyo[aa][bb][dd][ee] = -3000;
                        }
                        continue;
                    } // 110604
                } // dd220

                // Penalty for CHIGIRI
                if (m_aite_hakka_honsen == 0) {
                    if (config.p_t == 4)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 3 + chig_bb * 2 + chig_dd * 1) * 400;
                    if (config.p_t == 3)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 3 + chig_bb * 2 + chig_dd * 1) * 30;
                    if (config.p_t == 2)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (config.p_t == 1)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 1 + chig_bb * 1 + chig_dd * 1);
                } else {
                    if (config.p_t == 4)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (config.p_t == 3)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (config.p_t == 2)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (config.p_t == 1)
                        g_score_hukasa[aa][bb][dd] -= (chig_aa * 1 + chig_bb * 1 + chig_dd * 1);
                }

                // Penalty for field like 'U'
                if (ba_ee[2][10] != PuyoColor::EMPTY)
                    g_score_hukasa[aa][bb][dd] -= 1200;
                if (ba_ee[3][10] != PuyoColor::EMPTY)
                    g_score_hukasa[aa][bb][dd] -= 1200;
                for (int j = 0; j < 9; j++) {
                    if (ba_ee[0][j] == PuyoColor::EMPTY) {
                        if (ba_ee[1][j + 1] != PuyoColor::EMPTY)
                            g_score_hukasa[aa][bb][dd] -= 300;
                        break;
                    }
                }
                for (int i = 1; i < 5; i++) {
                    for (int j = 0; j < 9; j++) {
                        if (ba_ee[i][j] == PuyoColor::EMPTY) {
                            if ((ba_ee[i - 1][j + 1] != PuyoColor::EMPTY) && (ba_ee[i + 1][j + 1] != PuyoColor::EMPTY))
                                g_score_hukasa[aa][bb][dd] -= 300;
                            break;
                        }
                    }
                }
                for (int j = 0; j < 9; j++) {
                    if (ba_ee[5][j] == PuyoColor::EMPTY) {
                        if (ba_ee[4][j + 1] != PuyoColor::EMPTY)
                            g_score_hukasa[aa][bb][dd] -= 300;
                        break;
                    }
                }

                // GTR-check
                if ((zenkesi_own != 1) && (zenkesi_aite != 1) && (config.o_t)) {
                    if (m_hukks < 10)
                        g_score_hukasa[aa][bb][dd] += gtr(ba_ee);
                }

                // Balanced field
                for (int i = 0; i < 6; i++) {
                    for (int j = 0; j < 14; j++) {
                        if (ba_ee[i][j] == PuyoColor::EMPTY) {
                            teimen[i] = j;
                            break;
                        }
                    }
                }
                if ((zenkesi_aite != 1) && (config.r_t)) {
                    aveteimen = teimen[0] + teimen[1] + teimen[2] + teimen[3] + teimen[4] + teimen[5];
                    teimen[0] = (teimen[0] - 3) * 6 - aveteimen;
                    if (teimen[0] < 0)
                        teimen[0] = teimen[0] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[0] * 10;
                    teimen[1] = (teimen[1]) * 6 - aveteimen;
                    if (teimen[1] < 0)
                        teimen[1] = teimen[1] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[1] * 10;
                    teimen[2] = (teimen[2] + 1) * 6 - aveteimen;
                    if (teimen[2] < 0)
                        teimen[2] = teimen[2] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[2] * 10;
                    teimen[3] = (teimen[3] + 1) * 6 - aveteimen;
                    if (teimen[3] < 0)
                        teimen[3] = teimen[3] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[3] * 10;
                    teimen[4] = (teimen[4]) * 6 - aveteimen;
                    if (teimen[4] < 0)
                        teimen[4] = teimen[4] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[4] * 10;
                    teimen[5] = (teimen[5] - 3) * 6 - aveteimen;
                    if (teimen[5] < 0)
                        teimen[5] = teimen[5] * (-1);
                    g_score_hukasa[aa][bb][dd] -= teimen[5] * 10;
                }

                // Tanpatsu
                tanpatu_on = keshiko_aa * 2 + keshiko_bb;
                if (keshiko_aa < 5 && keshiko_bb < 5)
                    tanpatu_on = 0;
                if (keshiko_aa > 7 || keshiko_bb > 7)
                    tanpatu_on = 0;
                if (tanpatu_on > 0 && myf_kosuu_iro > aite_kosuu_iro + 1 && myf_kosuu_iro > 23 && m_one_tanpatu && m_aite_hakka_kosuu == 0 && m_aite_hakka_rensa == 0)
                    g_score_hukasa[aa][bb][dd] += tanpatu_on * 120;

                // TODO(peria): what ee means?
                for (int ee = 0; ee < EE_SIZE; ee++) {
                    if ((m_key_ee == 0) && (ee > 0))
                        break; // t2
                    int eex = 0;
                    if (ee == 0) {
                        eex = 0;
                        eecol = PuyoColor::EMPTY;
                    } else {
                        eex = (ee - 1) % 6;
                        int index = (ee - 1) / 6;
                        DCHECK_LE(0, index);
                        DCHECK_LT(index, static_cast<int>(ARRAY_SIZE(NORMAL_PUYO_COLORS)));
                        eecol = NORMAL_PUYO_COLORS[index];
                        if ((eex == 2) || (eex == 3))
                            continue;
                        if (ba_ee[0][11] != PuyoColor::EMPTY) {
                            if (eex == 0)
                                continue;
                        }
                        if (ba_ee[1][11] != PuyoColor::EMPTY) {
                            if ((eex == 0) || (eex == 1))
                                continue;
                        }
                        if (ba_ee[3][11] != PuyoColor::EMPTY) {
                            if ((eex == 4) || (eex == 5))
                                continue;
                        }
                        if (ba_ee[4][11] != PuyoColor::EMPTY) {
                            if ((eex == 4) || (eex == 5))
                                continue;
                        }
                        if (ba_ee[5][11] != PuyoColor::EMPTY) {
                            if (eex == 5)
                                continue;
                        }
                    }
                    PuyoColor ba[6][kHeight] {};
                    copyField(ba_ee, ba);
                    if (ee != 0) {
                        if (setti_puyo_1(ba, eex, eecol))
                            continue;
                    }

                    if (ba[2][11] != PuyoColor::EMPTY) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -3000;
                        continue;
                    }
                    if ((ba[0][10] == PuyoColor::EMPTY) && (ba[1][11] != PuyoColor::EMPTY) && (ba2[1][11] == PuyoColor::EMPTY)) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }
                    if (((ba[5][10] == PuyoColor::EMPTY) || (ba[4][10] == PuyoColor::EMPTY)) && (ba[3][11] != PuyoColor::EMPTY) && (ba2[3][11] == PuyoColor::EMPTY)) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }
                    if ((ba[5][10] == PuyoColor::EMPTY) && (ba[4][11] != PuyoColor::EMPTY) && (ba2[4][11] == PuyoColor::EMPTY)) {
                        g_chainhyk[aa][bb][dd][ee] = 0;
                        g_poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }

                    Check point2[6][12];
                    std::fill_n(&point2[0][0], 6 * 12, Check::Unknown);
                    UpdateAccessibility(ba, point2);

                    PuyoColor bass[6][kHeight] {};
                    int num2 = 3;
                    for (int i2 = 0; i2 < 6; i2++) {
                        hakkatakasa = 0;
                        for (int j2 = 0; j2 < 12; j2++) {
                            if (point2[i2][j2] == Check::Empty)
                                break;
                            if (point2[i2][j2] == Check::Unknown)
                                continue;
                            if (hakkatakasa < 6)
                                hakkatakasa++;
                            if (point2[i2][j2] == Check::Checked)
                                continue;
                            if ((num2 > 2))
                                copyField(ba, bass);
                            int num2 = 0;
                            int poi2s = 0;
                            int chain = 0;
                            Check tokus = point2[i2][j2];
                            saiki(bass, point2, i2, j2, &num2, bass[i2][j2]);
                            if (num2 >= 3) {
                                poi2s = j2 * config.takasa_point;
                                if (j2 > 5)
                                    poi2s += config.takasa_point;
                                if (tokus == Check::ColorWithEmptyUR ||
                                    tokus == Check::ColorWithEmptyUL ||
                                    tokus == Check::ColorWithEmptyU)
                                    poi2s += 100 * config.t_t;
                                if (zenkesi_aite == 1)
                                    poi2s = hakkatakasa * 300;

                                chain = chousei_syoukyo_3(bass, setti_basyo, &poi2s, &score_mm, tokus, i2, j2, config.ruiseki_point);

                                if ((dd < 220) && (m_myf_kosuu > 63))
                                    score_mm = score_mm * 1 / 2;
                                else if ((dd < 220) && (m_myf_kosuu > 61))
                                    score_mm = score_mm * 3 / 4;
                                else if ((dd < 220) && (m_myf_kosuu > 55))
                                    score_mm = score_mm * 6 / 7;

                            }
                            int pois = 0;
                            if (m_cchai <= chain) {
                                m_cchai = chain;
                                Check point[6][12] {};
                                num = 0;
                                for (int i = 0; i < 6; i++) {
                                    for (int j = 0; j < 12; j++) {
                                        if (bass[i][j] == PuyoColor::EMPTY)
                                            break;
                                        if (point[i][j] != Check::Checked && isNormalColor(bass[i][j])) {
                                            saiki(bass, point, i, j, &num, bass[i][j]);
                                            pois = pois + num * num * num;
                                            num = 0;
                                        }
                                    }
                                }
                            }

                            static const int yokotate = 4;
                            static const int yokopoint = 200;
                            if (chain > m_cchai - 3) {
                                for (int i = 0; i < 5; i++) {
                                    for (int j = 0; j < yokotate; j++) {
                                        if (isNormalColor(bass[i][j])) {
                                            if (bass[i][j] == bass[i + 1][j])
                                                pois += yokopoint;
                                        }
                                    }
                                } // 12-2
                            }
                            if (g_chainhyk[aa][bb][dd][ee] < chain) {
                                g_chainhyk[aa][bb][dd][ee] = chain;
                                g_poihyo[aa][bb][dd][ee] = (pois + poi2s);
                            } else if (g_chainhyk[aa][bb][dd][ee] == chain) {
                                if (g_poihyo[aa][bb][dd][ee] < (pois + poi2s)) {
                                    g_poihyo[aa][bb][dd][ee] = (pois + poi2s);
                                }
                            }
                            if (kuraichk == 1)
                                score_mm = 0;
                            if (score_mm > m_score_max) {
                                m_score_max = score_mm;
                                m_mmmax = aa;
                            }
                        }
                    }
                } // ee
            } // bb
        } // aa
    } // dd
    return 0;
}

}  // namespace test_lockit
