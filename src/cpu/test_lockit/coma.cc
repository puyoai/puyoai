#include "coma.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <sstream>
#include <string>

#include "color.h"
#include "field.h"

namespace test_lockit {

int chainhyk[22][22][221][EE_SIZE] {};
int poihyo[22][22][221][EE_SIZE] {};
int score_hukasa[22][22][221] {};

namespace {

bool IsMatchIndexAndColors(int id, int color[]) {
  static const int expect_colors[][2] = {
    {TL_RED, TL_RED},
    {TL_RED, TL_BLUE},
    {TL_RED, TL_YELLOW},
    {TL_RED, TL_GREEN},
    {TL_BLUE, TL_BLUE},
    {TL_BLUE, TL_YELLOW},
    {TL_BLUE, TL_GREEN},
    {TL_YELLOW, TL_YELLOW},
    {TL_YELLOW, TL_GREEN},
    {TL_GREEN, TL_GREEN},
  };

  if (color[0] == TL_UNKNOWN || color[1] == TL_UNKNOWN)
    return true;
  if (id == 220)
    return true;
  if (id >= 221 || id < 0)
    return false;

  id /= 22;
  return expect_colors[id][0] == color[0] && expect_colors[id][1] == color[1];
}

}  // namespace

void COMAI_HI::ref()
{
    cchai = 0;
    hukks = 0;
    conaa = 0;
    nexaa = 0;
    maxchais = 0;
    for (int i = 0; i < 22; i++) {
        para[i] = 0;
    }
    para[13] = 1;
    myf_kosuu = 0;
    saisoku_flag = saisoku_point;
    aite_hakka_rensa = 0;
    aite_hakka_nokori = 0;
    aite_hakka_zenkesi = 0;
    aite_hakka_kosuu = 0;
    nocc_aite_rensa_score = 0;
    aite_rensa_score = 0;
    aite_rensa_score_cc = 0;
    key_ee = 1;
    aite_hakkaji_score = 0;
    aite_hakka_jamako = 0;
    aite_hakka_honsen = 0;
    aite_puyo_uki = 0;
    aite_hakka_quick = 0;
    kougeki_on = 0;
    kougeki_edge = 0;
    kougeki_iryoku = 0;
    one_tanpatu = 1;
    score_max = 0;
    mmmax = -1;
    score_aa = -10;
    aa_max_score = 0;
    hakkatime = 0;
    numg = 0;
}

COMAI_HI::COMAI_HI()
{
    int i;
    cchai = 0;
    hukks = 0;
    conaa = 0;
    nexaa = 0;
    maxchais = 0;
    for (i = 0; i < 22; i++) {
        para[i] = 0;
    }
    myf_kosuu = 0;
    saisoku_flag = saisoku_point;
    aite_hakka_rensa = 0;
    aite_hakka_nokori = 0;
    aite_hakka_zenkesi = 0;
    aite_hakka_kosuu = 0;
    nocc_aite_rensa_score = 0;
    aite_rensa_score = 0;
    aite_rensa_score_cc = 0;
    key_ee = 1;
    aite_hakkaji_score = 0;
    aite_hakka_jamako = 0;
    aite_hakka_honsen = 0;
    aite_puyo_uki = 0;
    aite_hakka_quick = 0;
    kougeki_on = 0;
    kougeki_edge = 0;
    kougeki_iryoku = 0;
    one_tanpatu = 1;
    score_max = 0;
    mmmax = -1;
    score_aa = -10;
    aa_max_score = 0;
    hakkatime = 0;
    numg = 0;

    memset(katax, 0, sizeof(katax));
    memset(katay, 0, sizeof(katay));
    memset(kko, 0, sizeof(kko));
    memset(kinsi_a, 0, sizeof(kinsi_a));
    memset(kinsi_b, 0, sizeof(kinsi_b));
    memset(jakkin_a, 0, sizeof(jakkin_a));
    memset(jakkin_b, 0, sizeof(jakkin_b));
    memset(tankinko, 0, sizeof(tankinko));
    memset(jatako, 0, sizeof(jatako));
    read_template();
}

COMAI_HI::~COMAI_HI()
{
}

bool COMAI_HI::aite_attack_start(const int ba3[6][kHeight], int zenkesi_aite, int scos, int hakata)
{
    int ba[6][kHeight] {};
    int i, j;
    int kosuu_mae = 0, kosuu_ato = 0;
    int score = 0;
    int jamako_sabun;
    int quick = 0;
    bool ret_keshi = false;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            int c = ba3[i][j];
            ba[i][j] = c;
            if (IsColorPuyo(c))
                kosuu_mae++;
        }
    }

    aite_hakka_rensa = hon_syoukyo_score(ba, &score, &quick);
    aite_hakka_nokori = aite_hakka_rensa;
    hakkatime = hakata;
    if (aite_hakka_rensa > 0) {
        ret_keshi = true;
        aite_hakka_zenkesi = zenkesi_aite;
        jamako_sabun = aite_hakkaji_score / 70;
        aite_hakkaji_score = scos + score;
        aite_hakka_jamako = aite_hakkaji_score / 70 - jamako_sabun;
        aite_hakka_quick = quick;
        if (kougeki_on || kougeki_edge)
            aite_hakka_jamako -= kougeki_iryoku;
        if (aite_hakka_jamako < -11)
            aite_hakka_jamako += kougeki_iryoku;
        kougeki_on = 0;
        kougeki_iryoku = 0;
    }
    kougeki_edge = 0;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            if (IsColorPuyo(ba[i][j]))
                kosuu_ato++;
        }
    }

    aite_hakka_kosuu = kosuu_mae - kosuu_ato;
    if (aite_hakka_kosuu * 2 > kosuu_mae)
        aite_hakka_honsen = 1;

    return ret_keshi;
}

int COMAI_HI::aite_attack_nokori(const int [6][kHeight], int hakata)
{
    aite_hakka_nokori = aite_hakka_rensa - (hakata - hakkatime + 30) / 40;
    return 0;
}

int COMAI_HI::aite_rensa_end()
{
    aite_hakka_rensa = 0;
    aite_hakka_zenkesi = 0;
    aite_hakka_kosuu = 0;
    aite_hakka_honsen = 0;
    saisoku_flag = saisoku_point;
    aite_puyo_uki = 0;
    aite_hakka_quick = 0;
    aite_hakka_jamako = 0;
    aite_hakka_nokori = 0;
    return 0;
}

int COMAI_HI::aite_hyouka(const int ba3[6][kHeight], int tsumo[])
{
    int ba2[6][kHeight] {};
    int ba[6][kHeight] {};
    int ba_a[6][kHeight] {};
    int ba_b[6][kHeight] {};
    int ba_d[6][kHeight] {};
    int point[6][12];
    int i, j;
    int num;
    int n;
    int syo;
    int chain;
    int nx1, nx2, nn1, nn2;
    int nk1, nk2;

    int aa, bb, dd;
    int rakkaflg[6];
    int kiept[6];
    int keshiko_aa = 0;
    int keshiko_bb = 0;
    int keshiko_dd = 0;
    int keshikos;

    int setti_basyo[4];
    int dabuchk[20];
    int ichiren_kesi;
    int score_aa = 0;
    int score_bb = 0;
    int score_dd = 0;
    int scores = 0;
    int irokosuu = 0;

    int cc;
    int chain2 = 0;

    aite_rensa_score = 0;
    aite_rensa_score_cc = 0;
    nocc_aite_rensa_score = 0;

    nx1 = tsumo[0];
    nx2 = tsumo[1];
    nn1 = tsumo[2];
    nn2 = tsumo[3];
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            ba2[i][j] = ba3[i][j];
            if ((ba3[i][j] > 0) && (ba3[i][j] < 6))
                irokosuu++;
        }
    }

    for (aa = 0; aa < 22; aa++) {
        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;
        memcpy(ba_a, ba2, sizeof(ba));
        setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
        keshiko_aa = chousei_syoukyo_2(ba_a, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score_aa);
        if (ba_a[2][11] != 0)
            continue;
        for (bb = 0; bb < 22; bb++) {
            if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                continue;
            memcpy(ba_b, ba_a, sizeof(ba));
            setti_puyo(ba_b, bb, nn1, nn2, setti_basyo);
            keshiko_bb = chousei_syoukyo_2(ba_b, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score_bb);
            if (ba_b[2][11] != 0)
                continue;

            for (cc = 1; cc < 5; cc++) {
                nk1 = cc;
                nk2 = cc;
                for (dd = 0; dd < 6; dd++) {
                    if (tobashi_hantei_a(ba_b, dd, nk1, nk2))
                        continue;
                    memcpy(ba_d, ba_b, sizeof(ba));
                    setti_puyo(ba_d, dd, nk1, nk2, setti_basyo);
                    keshiko_dd = chousei_syoukyo_2(ba_d, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score_dd);
                    if (ba_d[2][11] != 0)
                        continue;

                    scores = score_aa;
                    keshikos = keshiko_aa;
                    if (scores < score_bb) {
                        scores = score_bb;
                        keshikos = keshiko_bb;
                    }
                    if (scores < score_dd) {
                        scores = score_dd;
                        keshikos = keshiko_dd;
                    }

                    if (aite_rensa_score < scores) {
                        aite_rensa_score = scores;
                    }
                    if (keshikos * 2 < irokosuu) {
                        if (nocc_aite_rensa_score < scores) {
                            nocc_aite_rensa_score = scores;
                            moni_kesiko[0] = keshiko_aa;
                            moni_iroko[0] = score_aa;
                            moni_kesiko[1] = keshiko_bb;
                            moni_iroko[1] = score_bb;
                            moni_kesiko[2] = keshiko_dd;
                            moni_iroko[2] = score_dd;
                        }
                    }
                } // dd
            } // cc

            // aaaaaaaa 111119
            for (cc = 0; cc < 25; cc++) {
                memcpy(ba, ba_b, sizeof(ba));
                if (cc == 0) {
                    continue;
                } else if (cc < 7) {
                    for (j = 0; j < 13; j++) {
                        if (ba[cc - 1][j] == 0) {
                            ba[cc - 1][j] = 1;
                            ba[cc - 1][j + 1] = 1;
                            break;
                        }
                    }
                } else if (cc < 13) {
                    for (j = 0; j < 13; j++) {
                        if (ba[cc - 7][j] == 0) {
                            ba[cc - 7][j] = 2;
                            ba[cc - 7][j + 1] = 2;
                            break;
                        }
                    }
                } else if (cc < 19) {
                    for (j = 0; j < 13; j++) {
                        if (ba[cc - 13][j] == 0) {
                            ba[cc - 13][j] = 3;
                            ba[cc - 13][j + 1] = 3;
                            break;
                        }
                    }
                } else if (cc < 25) {
                    for (j = 0; j < 13; j++) {
                        if (ba[cc - 19][j] == 0) {
                            ba[cc - 19][j] = 4;
                            ba[cc - 19][j + 1] = 4;
                            break;
                        }
                    }
                }

                if (ba[1][11] != 0) {
                    if ((cc % 6) == 1)
                        continue;
                }
                if (ba[2][11] != 0) {
                    if ((cc % 6) != 3)
                        continue;
                }
                if (ba[3][11] != 0) {
                    if ((cc % 6) == 5)
                        continue;
                    if (((cc % 6) == 0) && (cc != 0))
                        continue;
                }
                if (ba[4][11] != 0) {
                    if (((cc % 6) == 0) && (cc != 0))
                        continue;
                }

                syo = 1;
                chain2 = 0;
                num = 0;

                kiept[0] = 0;
                kiept[1] = 0;
                kiept[2] = 0;
                kiept[3] = 0;
                kiept[4] = 0;
                kiept[5] = 0;
                while (syo) {
                    syo = 0;
                    memset(point, 0, sizeof(point));
                    for (i = 0; i < 6; i++) {
                        for (j = kiept[i]; j < 12; j++) {
                            if (ba[i][j] == 0)
                                break;
                            if ((point[i][j] != 1) && (ba[i][j] != 6)) {
                                saiki(ba, point, i, j, &num, ba[i][j]);
                                point[i][j] = num;
                                num = 0;
                            }
                        }
                    }
                    rakkaflg[0] = 0;
                    rakkaflg[1] = 0;
                    rakkaflg[2] = 0;
                    rakkaflg[3] = 0;
                    rakkaflg[4] = 0;
                    rakkaflg[5] = 0;
                    for (i = 0; i < 6; i++) {
                        for (j = kiept[i]; j < 12; j++) {
                            if (point[i][j] > 3) {
                                syo = 1;
                                syou(ba, i, j, ba[i][j], rakkaflg);
                            }
                        }
                    }
                    for (i = 0; i < 6; i++) {
                        kiept[i] = 12;
                        if (rakkaflg[i] == 1) {
                            n = 0;
                            for (j = 0; j < 13; j++) {
                                if (ba[i][j] == 0) {
                                    if (n == 0)
                                        kiept[i] = j;
                                    n++;
                                } else if (n != 0) {
                                    ba[i][j - n] = ba[i][j];
                                    ba[i][j] = 0;
                                }
                            }
                        }
                    }
                    if (syo == 1)
                        chain2++;
                } // while
                if (aite_rensa_score_cc < chain2) {
                    aite_rensa_score_cc = chain2;
                }
            } // cc
        } // bb
    } // aa
    return 0;
}

int COMAI_HI::hyouka(const int ba3[6][kHeight], int tsumo[], int zenkesi_own, int aite_ba[6][kHeight], int zenkesi_aite)
{
    int ba[6][kHeight] {};
    int ba_a[6][kHeight] {};
    int ba2[6][kHeight] {};
    int point[6][12];
    int i, j;
    int num = 0;
    int n;
    int chain;
    int nx1, nx2, nn1, nn2;
    int aa, bb;
    int hym[22] {};
    int zenchk;
    int zenchain;
    int dabuchk[20] {};
    int hyktmp;
    int dd;
    int keschk = 0;
    int maxch = 0, maxach = 0;
    int saisoku;
    int kesu;
    int zenkes[22][22][22] {};
    int zenke[22] {};
    int setti_basyo[4];
    int myf_kosuu_kesi = 0, myf_kosuu_iro = 0;
    int kurai_large, kurai_middle, kurai_small;
    int aite_kosuu_iro = 0, kes2;
    int yokoyose = 2;
    int ichiren_kesi = 0;

    int ee;
    int ba_ee[6][kHeight];
    int keshiko_aa, keshiko_bb;
    int syuusoku = 0;

    int ccolor, cplace, coita, cyy = 0, nidub_point_a[22] = { 0 };
    int score = 0, maxscore = 0;
    int score_tmp;
    int kuraichk = 0;
    int quick = 0;
    int score_aonly = -10;
    int taiouchk = 0;
    int kurai_mini = 0;
    int score_tmp2, score_tai = -10, tai_max_score = 0;
    int wariko_taiou = 0;
    int chig_aa, chig_bb;
    int aite_ojama = 0;

    zenchk = 0;
    zenchain = 0;

    if (kougeki_on == 1) {
        kougeki_on = 0;
        kougeki_edge = 1;
    }

    myf_kosuu = 0;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            if (ba3[i][j] != 0) {
                myf_kosuu++;
            }
            if ((ba3[i][j] != 0) && (ba3[i][j] < 6)) {
                myf_kosuu_iro++;
            }
        }
    }
    kesu = int(myf_kosuu / 55);
    if (aite_hakka_rensa > 3)
        saisoku_flag = 0;
    saisoku = (saisoku_flag) && (myf_kosuu / 38);

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            if ((aite_ba[i][j] != 0) && (aite_ba[i][j] < 6)) {
                aite_kosuu_iro++;
            }
            if ((aite_ba[i][j] != 0)) {
                aite_ojama++;
            }
        }
    }
    aite_ojama = aite_ojama - aite_kosuu_iro;
    if (aite_ojama > 5)
        one_tanpatu = 0;
    if ((myf_kosuu_iro >= aite_kosuu_iro * 2) && (cchai > 6) && (aite_hakka_rensa < 4)) {
        kes2 = 1;
    } else {
        kes2 = 0;
    }

    nx1 = tsumo[0];
    nx2 = tsumo[1];
    nn1 = tsumo[2];
    nn2 = tsumo[3];
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            ba2[i][j] = ba3[i][j];
        }
    }

    // aaaaaaaaaaaaaaaaaaaaaa
    int maxpont[TM_TMNMUM][22] {};
    int maxp_matome[22] = { 0 };
    // TODO(peria): What |adubpt| means?
    // bool?
    int adubpt[22][22][22] {};
    int apos[22] = { 0 };

    tm_moni = 0;

    if (hukks < 255) {
        for (aa = 0; aa < 22; aa++) {
            if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                continue;
            memcpy(ba_a, ba2, sizeof(ba));
            setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
            if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                chig_aa = 1;
            else
                chig_aa = 0;
            keshiko_aa = chousei_syoukyo_2(ba_a, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score_tmp);
            if ((chain == 2) && (dabuchk[1] > 1)) {
                for (bb = 0; bb < 22; bb++) {
                    for (dd = 0; dd < 22; dd++) {
                        adubpt[aa][bb][dd] = 1;
                    }
                }
            }
            if (keshiko_aa != 0)
                continue;
            if (ba_a[2][11] != 0)
                continue;

            for (bb = 0; bb < 22; bb++) {
                if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                    continue;
                memcpy(ba_ee, ba_a, sizeof(ba));
                setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                    chig_bb = 1;
                else
                    chig_bb = 0;
                keshiko_bb = chousei_syoukyo_2(ba_ee, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score_tmp);
                if ((chain == 2) && (dabuchk[1] > 1)) {
                    for (dd = 0; dd < 22; dd++) {
                        adubpt[aa][bb][dd] = 1;
                    }
                }
                if (keshiko_bb != 0)
                    continue;
                if (ba_ee[2][11] != 0)
                    continue;
            }
        }
    } // hukks

    for (aa = 0; aa < 22; aa++) {
        for (i = 0; i < numg; i++) {
            if (maxp_matome[aa] < maxpont[i][aa])
                maxp_matome[aa] = maxpont[i][aa];
        }
    }
    for (aa = 0; aa < 22; aa++) {
        for (bb = 0; bb < 22; bb++) {
            for (dd = 0; dd < 22; dd++) {
                if (adubpt[aa][bb][dd] == 1)
                    apos[aa] = 1;
            }
        }
    }
    // aaaaaaaaaaaaaaaaaaaaaa

    for (aa = 0; aa < 22; aa++) {
        hym[aa] = -10000;
        zenke[aa] = 0;
    }

    kurai_large = 0;
    kurai_middle = 0;
    kurai_small = 0;
    if (aite_hakka_nokori < 3) {
        if ((((aite_hakka_rensa > 3) || (aite_hakka_kosuu > 15)) && (aite_hakka_jamako > 4))
            || ((aite_hakka_zenkesi == 1) && (aite_hakka_jamako > 35))
            || ((aite_hakka_zenkesi == 1) && (aite_hakka_jamako > 35))) {
            kurai_large = 1;
        } else if ((aite_hakka_kosuu > 12) && (aite_hakka_jamako > 4)) {
            kurai_middle = 1;
        } else if ((aite_hakka_kosuu > 8) && (aite_hakka_jamako > 4)) {
            kurai_small = 1;
        } else if ((aite_hakka_jamako > y_t)) {
            kurai_mini = 1;
        }
    }
    if (aite_hakka_nokori < 5) {
        if ((aite_hakka_rensa > 3) || (aite_hakka_kosuu > 15) || (aite_hakka_zenkesi == 1)) {
            syuusoku = 1;
        } else if ((aite_hakka_kosuu > 12)) {
            syuusoku = 1;
        }
    }

    wariko_taiou = 0;
    if ((((aite_hakka_rensa > 3) || (aite_hakka_kosuu > 15)) && (aite_hakka_jamako > 4))
        || ((aite_hakka_zenkesi == 1) && (zenkesi_own != 1))) {
        wariko_taiou = 1;
    } else if ((aite_hakka_kosuu > 12) && (aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((aite_hakka_kosuu > 8) && (aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((aite_hakka_jamako > 3)) {
        wariko_taiou = 1;
    }

    if (aite_hakka_rensa < 5) {
        if (myf_kosuu < 15) {
            for (aa = 0; aa < 22; aa++) {
                memcpy(ba_a, ba2, sizeof(ba_a));
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                    chig_aa = 1;
                else
                    chig_aa = 0;
                hon_syoukyo_score(ba_a, &score, &quick);
                if ((ba_a[0][0] == 0) && (ba_a[1][0] == 0) && (ba_a[2][0] == 0) && (ba_a[3][0] == 0)
                    && (ba_a[4][0] == 0) && (ba_a[5][0] == 0)) {
                    for (bb = 0; bb < 22; bb++) {
                        for (dd = 0; dd < 22; dd++) {
                            zenkes[aa][bb][dd] += score + 2100 - chig_aa * 3;
                        }
                    }
                }

                if (myf_kosuu < 13) {
                    for (bb = 0; bb < 22; bb++) {
                        memcpy(ba_ee, ba_a, sizeof(ba_ee));
                        setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                        if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                            chig_bb = 1;
                        else
                            chig_bb = 0;
                        hon_syoukyo_score(ba_ee, &score, &quick);
                        if ((ba_ee[0][0] == 0) && (ba_ee[1][0] == 0) && (ba_ee[2][0] == 0) && (ba_ee[3][0] == 0)
                            && (ba_ee[4][0] == 0) && (ba_ee[5][0] == 0)) {
                            for (dd = 0; dd < 22; dd++) {
                                zenkes[aa][bb][dd] += score + 2100 - (chig_aa + chig_bb) * 3;
                            }
                        }
                    }
                } // p<6
            }
        } // p<7
    }

    for (aa = 0; aa < 22; aa++) {
        for (bb = 0; bb < 22; bb++) {
            for (dd = 0; dd < 22; dd++) {
                if (zenchain <= zenkes[aa][bb][dd]) {
                    zenchain = zenkes[aa][bb][dd];
                    zenchk = aa;
                }
            }
        }
    }
    if (zenchain > 2100)
        zenke[zenchk] += 120000;

    num = 0;

    for (aa = 0; aa < 22; aa++) {

        for (i = 0; i < 20; i++) {
            dabuchk[i] = 0;
        }
        ichiren_kesi = 0;

        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;

        chain = 0;
        memcpy(ba, ba2, sizeof(ba));
        setti_puyo(ba, aa, nx1, nx2, setti_basyo);
        chain = hon_syoukyo_score(ba, &score, &quick);

        // つぶし
        if ((e_t) && (aite_hakka_rensa < 4)) {
            if ((chain == 2) && (score > aite_rensa_score + 260) && (score > 1600)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }

        if ((hukks > tubushiturn) && (aite_hakka_rensa < 4) && (aite_kosuu_iro > 12)) {
            if ((chain == 2) && (score > aite_rensa_score + 260) && (score > 690)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
            if (chain == 3 && aite_rensa_score_cc < 3) {
                hym[aa] += 72000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }
        if ((maxch <= chain) && (maxscore < score)) {
            maxch = chain;
            maxach = aa;
            maxscore = score;
        }
        myf_kosuu_kesi = 0;
        for (i = 0; i < 6; i++) {
            for (j = 0; j < 13; j++) {
                if ((ba[i][j] != 0) && (ba[i][j] < 6)) {
                    myf_kosuu_kesi++;
                }
            }
        }
        if ((chain == 1) && (score > 690) && (aite_hakka_rensa == 0)) {
            hym[aa] += 74000;
            keschk = 1;
        }
        // さいそく
        if (saisoku == 1) {
            if ((myf_kosuu_iro - myf_kosuu_kesi + 10 < myf_kosuu_kesi) && (score > nocc_aite_rensa_score + 130 * chain)
                && (chain > 1) && (aite_hakka_rensa == 0)) {
                hym[aa] += 73000;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }
        // kurai 対応
        if (u_t == 0) {
            if (((score > aite_hakka_jamako * 70 - 280) && (score < aite_hakka_jamako * 70 + 1400))
                && ((kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1)) && (aite_hakka_honsen == 0)
                && (chain > 1)) {
                hym[aa] += 120000;
                keschk = 1;
            }
        }
        if (zenkesi_own == 1) { // aaaa0909
            if ((chain == 1) && (aite_hakka_zenkesi == 1) && (aite_hakka_jamako < 36)) { // add121014
                hym[aa] += 74000;
            }
            if ((chain > 0) && (zenkesi_aite != 1)) {
                hym[aa] += 74000;
            }
        }
    } // aa
    if (kes2 == 1) {
        if ((maxch > cchai) && (cchai > 1)) {
            hym[maxach] += 150000;
            keschk = 1;
        } else if ((maxch > cchai - 1) && (cchai - 1 > 1)) {
            hym[maxach] += 148000;
            keschk = 1;
        } else if ((maxch > cchai - 2) && (cchai - 2 > 1)) {
            hym[maxach] += 146000;
            keschk = 1;
        }
    }
    if ((wariko_taiou == 1) && (aite_hakka_honsen == 0) && (keschk == 0)) {
        wariko_taiou = 1;
    } else {
        wariko_taiou = 0;
    }

    if (((kurai_mini == 1) || (kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1))
        && (aite_hakka_honsen == 0) && (keschk == 0)) {
        taiouchk = 1;
    }
    if (((kurai_mini == 1) && (keschk == 0) && (myf_kosuu > 64)) || ((kurai_large == 1) && (keschk == 0))
        || ((kurai_middle == 1) && (keschk == 0)) || ((kurai_small == 1) && (keschk == 0))
        || ((aite_hakka_honsen == 1) && (aite_hakka_nokori < 3))) {
        kuraichk = 1;
    }

    num = 0;
    if ((aite_hakka_honsen == 0) && (q_t)) {
        if (myf_kosuu > 23) {
            for (aa = 0; aa < 22; aa++) {
                if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                    continue;
                memcpy(ba_a, ba2, sizeof(ba));
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                if (chousei_syoukyo(ba_a, setti_basyo) != 0)
                    continue;
                for (cplace = 0; cplace < 6; cplace++) {
                    if (ba_a[0][11] != 0) {
                        if (cplace == 0)
                            continue;
                    }
                    if (ba_a[1][11] != 0) {
                        if ((cplace == 0) || (cplace == 1))
                            continue;
                    }
                    if (ba_a[3][11] != 0) {
                        if ((cplace == 3) || (cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[4][11] != 0) {
                        if ((cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[5][11] != 0) {
                        if (cplace == 5)
                            continue;
                    }
                    for (cyy = 0; cyy < 2; cyy++) {
                        for (ccolor = 1; ccolor < 5; ccolor++) {
                            memcpy(ba, ba_a, sizeof(ba));
                            coita = 0;
                            for (j = 0; j < (12 - cyy); j++) {
                                if (ba[cplace][j + cyy] == 0) {
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
                            chain = 0;
                            chousei_syoukyo_2(ba, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score);
                            if (is_2dub_cpu) {
                                if ((myf_kosuu < 46) && (chain == 2) && (dabuchk[1] > 1))
                                    nidub_point_a[aa] = 3000;
                            } else {
                                if ((myf_kosuu < 46) && (chain == 2) && (dabuchk[1] > 1))
                                    nidub_point_a[aa] = 1200;
                            }
                        }
                    } // cyy
                } // cc
            } // aa
        }

    } // aite_hakka_honsen==0

    if (a_t == 0)
        wariko_taiou = taiouchk;

    cchai = 0;

    for (aa = 0; aa < 22; aa++) {
        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;
        memcpy(ba_a, ba2, sizeof(ba));
        setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
        if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
            chig_aa = 1;
        else
            chig_aa = 0;
        keshiko_aa = chousei_syoukyo_sc(ba_a, setti_basyo, &score_tmp);
        score_aonly = score_tmp; // only
        score_tmp2 = score_tmp;
        if ((ba_a[2][11] == 0) && (score_tmp > score_aa)) {
            score_aa = score_tmp;
            aa_max_score = aa;
            tesuu_mon = 1;
        }
        if ((ba_a[2][11] == 0) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
            && (aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > aite_hakka_jamako * 70)) {
            if ((myf_kosuu_iro - 1 > keshiko_aa * 2) || (i_t)) {
                score_tai = score_tmp2;
                tai_max_score = aa;
            }
        }
        if ((myf_kosuu_iro - keshiko_aa + 8) < cchai * 4)
            continue;
        if (ba_a[2][11] != 0) {
            // In case the 1st control leads to die, do not score following controls.
            for (bb = 0; bb < 22; bb++) {
                for (dd = 0; dd < 221; dd++) {
                    for (ee = 0; ee < EE_SIZE; ee++) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
            }
            continue;
        } // 110604

        for (bb = 0; bb < 22; bb++) {
            if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                continue;
            memcpy(ba_ee, ba_a, sizeof(ba));
            setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
            if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                chig_bb = 1;
            else
                chig_bb = 0;
            keshiko_bb = chousei_syoukyo_sc(ba_ee, setti_basyo, &score_tmp);
            score_tmp2 = score_tmp;
            if ((kuraichk == 1) && (aite_hakka_nokori < 2) && (score_aonly > 0))
                score_tmp = 0; // only
            if ((kuraichk == 1) && ((aite_hakka_nokori < 1)
                                    || ((aite_hakka_nokori < 2) && ((aite_puyo_uki == 0) && (aite_hakka_quick == 1)))))
                score_tmp = 0; // 0225
            if ((ba_ee[2][11] == 0) && (score_tmp > score_aa)) {
                score_aa = score_tmp;
                aa_max_score = aa;
                tesuu_mon = 2;
            }
            if ((wariko_taiou == 1) && (aite_hakka_nokori < 2) && (score_aonly > 0))
                score_tmp2 = 0; // only
            if ((wariko_taiou == 1)
                && ((aite_hakka_nokori < 1)
                    || ((aite_hakka_nokori < 2) && ((aite_puyo_uki == 0) && (aite_hakka_quick == 1)))))
                score_tmp2 = 0; // 0225
            if ((ba_ee[2][11] == 0) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                && (aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > aite_hakka_jamako * 70)) {
                if ((myf_kosuu_iro + 1 > (keshiko_aa + keshiko_bb) * 2) || (i_t)) {
                    score_tai = score_tmp2;
                    tai_max_score = aa;
                }
            }
            if ((myf_kosuu_iro - keshiko_aa - keshiko_bb + 8) < cchai * 4)
                continue;
            if (ba_ee[2][11] != 0) {
                for (ee = 0; ee < EE_SIZE; ee++) {
                    for (dd = 0; dd < 221; dd++) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
                continue;
            } // 110604
        } // bb
    } // aa

    for (aa = 0; aa < 22; aa++) {
        for (bb = 0; bb < 22; bb++) {
            for (dd = 0; dd < 221; dd++) {
                // TODO(peria): Skip some cases if next2 tsumo is known.
                if (!IsMatchIndexAndColors(dd, &tsumo[4]))
                    continue;
                for (ee = 0; ee < EE_SIZE; ee++) {
                    if ((key_ee == 0) && (ee > 0))
                        break; // t2
                    hyktmp = chainhyk[aa][bb][dd][ee] * 1000 + poihyo[aa][bb][dd][ee]
                             + (((myf_kosuu > 40) || syuusoku) * (ee == 0) * 300) + score_hukasa[aa][bb][dd];
                    if (hym[aa] < hyktmp) {
                        hym[aa] = hyktmp;
                        max_ee = ee;
                    }
                }
            }
        }
    }
    if (((myf_kosuu > 50) || (syuusoku == 1)) && (max_ee == 0))
        key_ee = 0;
    if ((myf_kosuu < 44) && (syuusoku == 0))
        key_ee = 1;
    for (aa = 0; aa < 22; aa++) {
        hym[aa] += zenke[aa];
        hym[aa] += nidub_point_a[aa];
    }
    kuraichk_mon = kuraichk;
    score_mon = -1;
    if (((kesu == 1) && (score_max < score_aa)) || ((kuraichk == 1) && (score_aa > 530))) {
        hym[aa_max_score] += 100000;
        score_mon = score_aa;
    } else if ((kesu == 1) && (mmmax != -1)) {
        hym[mmmax] += 60000;
        score_mon = score_aa;
    }
    if (u_t == 1) {
        if ((wariko_taiou == 1) && (((score_tai > 270) && ((myf_kosuu > 36) || (ba2[2][10] != 0)
                                                           || (aite_hakka_zenkesi == 1))) || (score_tai > 840))) {
            hym[tai_max_score] += 140000;
            score_mon = score_tai;
        }
    }

    // 低い場所
    for (i = 0; i < 6; i++) {
        n = 0;
        for (j = 0; j < 13; j++) {
            if (ba2[i][j] == 0)
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
    if (((zenkesi_own == 1) && (zenkesi_aite != 1)) || ((aite_hakka_honsen == 0) && (w_t)) || (zenkesi_aite == 1)) {
        for (aa = 0; aa < 22; aa++) {
            memcpy(ba, ba2, sizeof(ba));
            if (aa < 6) {
                for (j = 0; j < 13; j++) {
                    if (ba[aa][j] == 0) {
                        ba[aa][j] = nx1;
                        ba[aa][j + 1] = nx2;
                        break;
                    }
                }
            } else if (aa < 12) {
                for (j = 0; j < 13; j++) {
                    if (ba[aa - 6][j] == 0) {
                        ba[aa - 6][j] = nx2;
                        ba[aa - 6][j + 1] = nx1;
                        break;
                    }
                }
            } else if (aa < 17) {
                for (j = 0; j < 13; j++) {
                    if (ba[aa - 11][j] == 0) {
                        ba[aa - 11][j] = nx1;
                        break;
                    }
                }
                for (j = 0; j < 13; j++) {
                    if (ba[aa - 12][j] == 0) {
                        ba[aa - 12][j] = nx2;
                        break;
                    }
                }
            } else if (aa < 22) {
                for (j = 0; j < 13; j++) {
                    if (ba[aa - 17][j] == 0) {
                        ba[aa - 17][j] = nx1;
                        break;
                    }
                }
                for (j = 0; j < 13; j++) {
                    if (ba[aa - 16][j] == 0) {
                        ba[aa - 16][j] = nx2;
                        break;
                    }
                }
            }
            for (i = 0; i < 6; i++) {
                for (j = 0; j < 12; j++) {
                    point[i][j] = 0;
                }
            }
            num = 0;
            for (i = 0; i < 6; i++) {
                for (j = 0; j < 12; j++) {
                    if (ba[i][j] == 0)
                        break;
                    if ((point[i][j] != 1) && (ba[i][j] != 6)) {
                        saiki(ba, point, i, j, &num, ba[i][j]);
                        if (num != 0) {
                            if (num == 2)
                                hym[aa] += 30 * renketu_bairitu;
                            if (num == 3)
                                hym[aa] += 120 * renketu_bairitu;
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

    for (aa = 0; aa < 22; aa++) {
        if ((myf_kosuu < 52) && (myf_kosuu_iro > 23) && (aite_hakka_rensa < 4) && (aite_hakka_honsen == 0))
            hym[aa] += apos[aa] * 2000;
        if (is_2dub_cpu) {
            if ((myf_kosuu < 52) && (aite_hakka_rensa < 4) && (aite_hakka_honsen == 0))
                hym[aa] += apos[aa] * 2000;
        }
    }

    for (aa = 0; aa < 22; aa++) {
        if (aa < 6)
            para[aa] = hym[aa] * 10 + 4;
        else if (aa < 12)
            para[aa + 5] = hym[aa] * 10 + 3;
        else if (aa < 17)
            para[aa + 5] = hym[aa] * 10 + 2;
        else if (aa < 22)
            para[aa - 11] = hym[aa] * 10 + 1;
    }
    return 0;
}

int COMAI_HI::saiki(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_right(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_left(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_up(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    return 0;
}
int COMAI_HI::saiki_down(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::syou(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_right(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_left(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_up(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    return 0;
}
int COMAI_HI::syou_down(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_downx(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    *num += 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_right_num(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_left_num(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}
int COMAI_HI::syou_up_num(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    return 0;
}
int COMAI_HI::syou_down_num(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}

int COMAI_HI::saiki_3(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_3_right(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_3_left(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
    return 0;
}
int COMAI_HI::saiki_3_up(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    return 0;
}
int COMAI_HI::saiki_3_down(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
    return 0;
}

int COMAI_HI::syou_2(int ba[][kHeight], int x, int y, int incol)
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_2(ba, x - 1, y, incol);
    if ((x != 0) && (ba[x - 1][y] == 6))
        ba[x - 1][y] = 0;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_2(ba, x, y + 1, incol);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_2(ba, x + 1, y, incol);
    if ((x != 5) && (ba[x + 1][y] == 6))
        ba[x + 1][y] = 0;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_2(ba, x, y - 1, incol);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
    return 0;
}

int COMAI_HI::tobashi_hantei_a(const int ba2[][kHeight], int aa, int nx1, int nx2)
{
    if (nx1 == nx2) {
        if (((aa > 5) && (aa < 12)) || ((aa > 13) && (aa < 19)))
            return 1;
    }
    if (ba2[0][11] != 0) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
    }
    if (ba2[1][11] != 0) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
        if ((aa == 1) || (aa == 7) || (aa == 13) || (aa == 18))
            return 1;
    }
    if (ba2[3][11] != 0) {
        if ((aa == 3) || (aa == 9) || (aa == 14) || (aa == 19))
            return 1;
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[4][11] != 0) {
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[5][11] != 0) {
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    return 0;
}

int COMAI_HI::setti_puyo(int ba[][kHeight], int aa, int nx1, int nx2, int setti_basyo[])
{
    int j;
    if (aa < 6) {
        for (j = 0; j < 13; j++) {
            if (ba[aa][j] == 0) {
                ba[aa][j] = nx1;
                ba[aa][j + 1] = nx2;
                setti_basyo[0] = aa;
                setti_basyo[1] = j;
                setti_basyo[2] = aa;
                setti_basyo[3] = j + 1;
                break;
            }
        }
    } else if (aa < 12) {
        for (j = 0; j < 13; j++) {
            if (ba[aa - 6][j] == 0) {
                ba[aa - 6][j] = nx2;
                ba[aa - 6][j + 1] = nx1;
                setti_basyo[0] = aa - 6;
                setti_basyo[1] = j;
                setti_basyo[2] = aa - 6;
                setti_basyo[3] = j + 1;
                break;
            }
        }
    } else if (aa < 17) {
        for (j = 0; j < 13; j++) {
            if (ba[aa - 11][j] == 0) {
                ba[aa - 11][j] = nx1;
                setti_basyo[0] = aa - 11;
                setti_basyo[1] = j;
                break;
            }
        }
        for (j = 0; j < 13; j++) {
            if (ba[aa - 12][j] == 0) {
                ba[aa - 12][j] = nx2;
                setti_basyo[2] = aa - 12;
                setti_basyo[3] = j;
                break;
            }
        }
    } else if (aa < 22) {
        for (j = 0; j < 13; j++) {
            if (ba[aa - 17][j] == 0) {
                ba[aa - 17][j] = nx1;
                setti_basyo[0] = aa - 17;
                setti_basyo[1] = j;
                break;
            }
        }
        for (j = 0; j < 13; j++) {
            if (ba[aa - 16][j] == 0) {
                ba[aa - 16][j] = nx2;
                setti_basyo[2] = aa - 16;
                setti_basyo[3] = j;
                break;
            }
        }
    }
    return 0;
}

int COMAI_HI::tobashi_hantei_b(const int ba2[][kHeight], int aa)
{
    if (ba2[0][11] != 0) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
    }
    if (ba2[1][11] != 0) {
        if ((aa == 0) || (aa == 6) || (aa == 12) || (aa == 17))
            return 1;
        if ((aa == 1) || (aa == 7) || (aa == 13) || (aa == 18))
            return 1;
    }
    if (ba2[3][11] != 0) {
        if ((aa == 3) || (aa == 9) || (aa == 14) || (aa == 19))
            return 1;
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[4][11] != 0) {
        if ((aa == 4) || (aa == 10) || (aa == 15) || (aa == 20))
            return 1;
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    if (ba2[5][11] != 0) {
        if ((aa == 5) || (aa == 11) || (aa == 16) || (aa == 21))
            return 1;
    }
    return 0;
}

int COMAI_HI::chousei_syoukyo(int ba[][kHeight], int setti_basyo[])
{
    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == 0) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = 0;
                }
            }
        }
    }

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == 0)
                    break;
                if (ba[i][j] != 6) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = 0;
                    }
                }
            }
        }
    }

    return keshiko;
}

int COMAI_HI::hon_syoukyo(int ba[][kHeight])
{
    int num = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6];
    int n;
    int chain = 0;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == 0)
                    break;
                if (ba[i][j] != 6) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = 0;
                    }
                }
            }
        }
        if (syo == 1)
            chain++;
    }
    return chain;
}

int COMAI_HI::hon_syoukyo_score(int ba[][kHeight], int* score, int* quick)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6];
    int n;
    int chain = 0;

    (*quick) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == 0)
                    break;
                if (ba[i][j] != 6) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                    }
                    num = 0;
                }
            }
        }
        if (syo == 1)
            (*quick) = 1;
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = 0;
                        (*quick) = 0;
                    }
                }
            }
        }
        if (syo == 1)
            chain++;
    }

    *score = 0;
    for (i = 0; i < chain; i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    return chain;
}

int COMAI_HI::setti_puyo_1(int ba[][kHeight], int eex, int eecol)
{
    int j;
    int oita = 0;
    int num = 0;
    int setti_basyoy;
    for (j = 0; j < 12; j++) {
        if (ba[eex][j] == 0) {
            ba[eex][j] = eecol;
            setti_basyoy = j;
            oita = 1;
            break;
        }
    }
    if (oita == 0)
        return 1;
    saiki_4(ba, eex, setti_basyoy, &num, eecol);
    if (num > 3)
        return 1;

    return 0;
}
int COMAI_HI::saiki_4(int ba[][kHeight], int x, int y, int* num, int incol)
{
    ba[x][y] = 0;
    *num += 1;
    if (*num > 3) {
        ba[x][y] = incol;
        return 0;
    }
    if ((x != 0) && (incol == ba[x - 1][y]))
        saiki_4(ba, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]))
        saiki_4(ba, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]))
        saiki_4(ba, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]))
        saiki_4(ba, x, y - 1, num, incol);
    ba[x][y] = incol;
    return 0;
}

int COMAI_HI::chousei_syoukyo_2(int ba[][kHeight], int setti_basyo[], int* chain, int dabuchk[], int* ichiren_kesi, int* score)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;
    for (i = 0; i < 20; i++) {
        dabuchk[i] = 0;
    }
    (*ichiren_kesi) = 0;
    *score = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    (*chain) = 0;
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        color = ba[a][b];
        renketsu[*chain][color - 1] += numa;
        if (numa > 10)
            renketsubonus[*chain] += 10; // bugggggg 111102
        else if (numa > 4)
            renketsubonus[*chain] += numa - 3;
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        color = ba[c][d];
        renketsu[*chain][color - 1] += numb;
        if (numb > 10)
            renketsubonus[*chain] += 10; // bugggggg 111102
        else if (numb > 4)
            renketsubonus[*chain] += numb - 3;
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == 0) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = 0;
                }
            }
        }
    }
    (*chain) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (ba[i][j] == 0)
                    continue;
                if ((point[i][j] != 1) && (ba[i][j] != 6)) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[*chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[*chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[*chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
                        dabuchk[*chain]++;
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = 0;
                    }
                }
            }
        }
        if (syo == 1)
            *chain += 1;
    }
    for (i = 0; i < (*chain); i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    return keshiko;
}

int COMAI_HI::chousei_syoukyo_sc(int ba[][kHeight], int setti_basyo[], int* score)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;
    int chain;
    *score = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    (chain) = 0;
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        color = ba[a][b];
        renketsu[chain][color - 1] += numa;
        if (numa > 10)
            renketsubonus[chain] += 10; // bugggggg 111102
        else if (numa > 4)
            renketsubonus[chain] += numa - 3;
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        color = ba[c][d];
        renketsu[chain][color - 1] += numb;
        if (numb > 10)
            renketsubonus[chain] += 10; // bugggggg 111102
        else if (numb > 4)
            renketsubonus[chain] += numb - 3;
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == 0) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = 0;
                }
            }
        }
    }
    (chain) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (ba[i][j] == 0)
                    continue;
                if ((point[i][j] != 1) && (ba[i][j] != 6)) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = 0;
                    }
                }
            }
        }
        if (syo == 1)
            chain += 1;
    }
    for (i = 0; i < (chain); i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    return keshiko;
}

int COMAI_HI::chousei_syoukyo_3(int bass[][kHeight], int[], int* poi2s, int* score, int tokus, int i2, int j2)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int chain = 1;
    int rakka_ruiseki = 0;
    *score = 0;
    rakkaflg[0] = 0;
    rakkaflg[1] = 0;
    rakkaflg[2] = 0;
    rakkaflg[3] = 0;
    rakkaflg[4] = 0;
    rakkaflg[5] = 0;
    color = bass[i2][j2];
    if (tokus < 5) {
        syou_downx(bass, i2, j2 + 1, bass[i2][j2], rakkaflg, &num);
    } else if (tokus < 7) {
        syou_downx(bass, i2 + 1, j2, bass[i2][j2], rakkaflg, &num);
    } else if (tokus == 7) {
        syou_downx(bass, i2 - 1, j2, bass[i2][j2], rakkaflg, &num);
    }
    renketsu[0][color - 1] = num;
    if (num > 10)
        renketsubonus[0] += 10; // bugggggg 111102
    else if (num > 4)
        renketsubonus[0] = num - 3;

    num = 0;

    kiept[0] = 0;
    kiept[1] = 0;
    kiept[2] = 0;
    kiept[3] = 0;
    kiept[4] = 0;
    kiept[5] = 0;
    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (bass[i][j] == 0) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    bass[i][j - n] = bass[i][j];
                    bass[i][j] = 0;
                }
            }
            rakka_ruiseki += n;
        }
    }

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (bass[i][j] == 0)
                    break;
                if (bass[i][j] != 6) {
                    saiki(bass, point, i, j, &num, bass[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = bass[i][j];
                        renketsu[chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        (*poi2s) = (*poi2s) - num * num;
                        syou(bass, i, j, bass[i][j], rakkaflg);
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (bass[i][j] == 0) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        bass[i][j - n] = bass[i][j];
                        bass[i][j] = 0;
                    }
                }
                rakka_ruiseki += n;
            }
        }
        chain++;
    } // while
    chain--;

    for (i = 0; i < (chain); i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    (*poi2s) = (*poi2s) - rakka_ruiseki * ruiseki_point;
    return chain;
}

int COMAI_HI::gtr(const int f[][kHeight])
{
    int sc = 0;

    if ((f[0][0] != 0) && (f[1][0] != 0)) {
        if (f[0][0] == f[1][0]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][0] != 0) && (f[1][2] != 0)) {
        if (f[0][0] == f[1][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][0] != 0) && (f[2][1] != 0)) {
        if (f[0][0] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][0] != 0) && (f[1][2] != 0)) {
        if (f[1][0] == f[1][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][0] != 0) && (f[2][1] != 0)) {
        if (f[1][0] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[2][0] != 0) && (f[2][1] != 0)) {
        if (f[1][2] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }

    if ((f[0][1] != 0) && (f[0][2] != 0)) {
        if (f[0][1] == f[0][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][1] != 0) && (f[1][1] != 0)) {
        if (f[0][1] == f[1][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][1] != 0) && (f[0][2] != 0)) {
        if (f[1][1] == f[0][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }

    if ((f[0][2] != 0) && (f[1][3] != 0)) {
        if (f[0][2] == f[0][3]) {
            sc -= 2000;
        }
    }

    if ((f[1][2] != 0) && (f[1][3] != 0)) {
        if (f[1][2] == f[1][3]) {
            sc -= 500;
        }
    }
    if ((f[1][2] != 0) && (f[2][2] != 0)) {
        if (f[1][2] == f[2][2]) {
            sc -= 500;
        }
    }
    if ((f[1][0] != 0) && (f[2][0] != 0)) {
        if (f[1][0] == f[2][0]) {
            sc -= 1000;
        }
    }
    if ((f[2][1] != 0) && (f[2][0] != 0)) {
        if (f[2][1] == f[2][0]) {
            sc -= 1000;
        }
    }
    if ((f[2][1] != 0) && (f[2][2] != 0)) {
        if (f[2][1] == f[2][2]) {
            sc -= 500;
        }
    }

    return sc;
}

int COMAI_HI::setti_ojama(int f[][kHeight], int ojamako)
{
    int i, j;
    int cnt;
    int okiko;

    okiko = (ojamako + 3) / 6;

    for (i = 0; i < 6; i++) {
        cnt = 0;
        for (j = 0; j < 13; j++) {
            if (f[i][j] == 0) {
                f[i][j] = 6;
                cnt++;
            }
            if (cnt == okiko)
                break;
        }
    }
    return 0;
}

int COMAI_HI::read_template()
{
    int i, j;
    FILE* fp;
    char str[256];
    int cnt = 0;
    int a[78];

    if ((fp = fopen("rensa.dat", "r")) == NULL) {
        return 0;
    }
    i = 0;

    while (1) {
        if (cnt == 0) {
            if ((fscanf(fp, "%d", &numg)) == EOF)
                break;
            colko[i] = numg + 1;
            if (numg >= TM_COLNUM)
                break;
        } else if (cnt == 1) {
            for (j = 1; j < colko[i]; j++) {
                if ((fscanf(fp, "%d%*c", &colscore[i][j])) == EOF)
                    break;
            }
        } else if (cnt == 2) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6],
                        &a[7], &a[8], &a[9], &a[10], &a[11], &a[12])) == EOF)
                break;
        } else if (cnt == 3) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[13], &a[14], &a[15], &a[16], &a[17], &a[18],
                        &a[19], &a[20], &a[21], &a[22], &a[23], &a[24], &a[25])) == EOF)
                break;
        } else if (cnt == 4) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[26], &a[27], &a[28], &a[29], &a[30], &a[31],
                        &a[32], &a[33], &a[34], &a[35], &a[36], &a[37], &a[38])) == EOF)
                break;
        } else if (cnt == 5) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[39], &a[40], &a[41], &a[42], &a[43], &a[44],
                        &a[45], &a[46], &a[47], &a[48], &a[49], &a[50], &a[51])) == EOF)
                break;
        } else if (cnt == 6) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[52], &a[53], &a[54], &a[55], &a[56], &a[57],
                        &a[58], &a[59], &a[60], &a[61], &a[62], &a[63], &a[64])) == EOF)
                break;
        } else if (cnt == 7) {
            if ((fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &a[65], &a[66], &a[67], &a[68], &a[69], &a[70],
                        &a[71], &a[72], &a[73], &a[74], &a[75], &a[76], &a[77])) == EOF)
                break;
            for (j = 0; j < 78; j++) {
                if ((a[j] > 0) && (a[j] < TM_COLNUM)) {
                    katax[i][a[j]][kko[i][a[j]]] = j / 13;
                    katay[i][a[j]][kko[i][a[j]]] = j % 13;
                    kko[i][a[j]]++;
                } else if (a[j] == -1) {
                    tankinx[i][tankinko[i]] = j / 13;
                    tankiny[i][tankinko[i]] = j % 13;
                    tankinko[i]++;
                } else if (a[j] > 100) {
                    jatax[i][a[j] - 101] = j / 13;
                    jatay[i][a[j] - 101] = j % 13;
                    jatako[i]++;
                }
            }
        } else if (cnt == 8) {
            if ((fscanf(fp, "%d", &numg)) == EOF)
                break;
            kinko[i] = numg;
            if (numg > TM_KINPT)
                break;
        } else if (cnt == 9) {
            for (j = 0; j < kinko[i]; j++) {
                if ((fscanf(fp, "%d%*c", &kinsi_a[i][j])) == EOF)
                    break;
                if ((fscanf(fp, "%d%*c", &kinsi_b[i][j])) == EOF)
                    break;
            }
        } else if (cnt == 10) {
            if ((fscanf(fp, "%d", &numg)) == EOF)
                break;
            jakkinko[i] = numg;
            if (numg > TM_JAKKINPT)
                break;
        } else if (cnt == 11) {
            for (j = 0; j < jakkinko[i]; j++) {
                if ((fscanf(fp, "%d%*c", &jakkin_a[i][j])) == EOF)
                    break;
                if ((fscanf(fp, "%d%*c", &jakkin_b[i][j])) == EOF)
                    break;
            }
        } else if (cnt == 12) {
            if ((fscanf(fp, "%d", &numg)) == EOF)
                break;
            tm_turn[i] = numg;
        } else if (cnt == 13) {
            if ((fscanf(fp, "%s", str)) == EOF)
                break;
            if (strcmp(str, "####################") == 0)
                i++;
            else
                break;
            cnt = -1;
        }
        cnt++;
    }
    numg = i;
    fclose(fp);
    return numg;
}

int COMAI_HI::pre_hyouka(const int ba3[6][kHeight], int tsumo[], int zenkesi_own, int aite_ba[6][kHeight],
                         int zenkesi_aite, int fast)
{
    int ba[6][kHeight] {};
    int ba_a[6][kHeight] {};
    int ba2[6][kHeight] {};
    int bass[6][kHeight];
    int point[6][12];
    int point2[6][12] = {
        {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}};
    int i, j;
    int num = 0;
    int chain;
    int nx1, nx2, nn1, nn2;
    int aa, bb;
    int pois, poi2s;
    int hym[22];
    int zenchk;
    int zenchain;
    int dabuchk[20];
    int i2, j2;
    int dd, nk1, nk2, num2;
    int keschk = 0;
    int maxch = 0, maxach;
    int tokus;
    int saisoku;
    int teimen[6];
    int zenkes[22][22][22] {};
    int zenke[22] = { 0 };
    int setti_basyo[4];
    int myf_kosuu_kesi = 0, myf_kosuu_iro = 0;
    int kurai_large, kurai_middle, kurai_small;
    int aite_kosuu_iro = 0, kes2;
    int yokotate = 4;
    int yokopoint = 200;
    int ichiren_kesi = 0;

    int ee, eex, eecol;
    int ba_ee[6][kHeight];
    int keshiko_aa, keshiko_bb, keshiko_dd;

    int ccolor, cplace, coita, cyy = 0;
    int score = 0, maxscore = 0;
    int score_tmp, score_mm = 0;
    int kuraichk = 0;
    int quick = 0;
    int score_aonly = -10, score_bonly = -10;
    int taiouchk = 0;
    int kurai_mini = 0;
    int score_tmp2, score_tai = -10;
    int aveteimen = 0;
    int hakkatakasa = 0;
    int wariko_taiou = 0;
    int chig_aa, chig_bb, chig_dd;
    int aite_ojama = 0;
    int tanpatu_on = 0;

    hukks++;

    score_max = 0;
    mmmax = -1;
    score_aa = -10;
    aa_max_score = 0;
    zenchk = 0;
    zenchain = 0;

    if (kougeki_on == 1) {
        kougeki_on = 0;
        kougeki_edge = 1;
    }

    myf_kosuu = 0;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            if (ba3[i][j] != 0) {
                myf_kosuu++;
            }
            if ((ba3[i][j] != 0) && (ba3[i][j] < 6)) {
                myf_kosuu_iro++;
            }
        }
    }

    if (aite_hakka_rensa > 3)
        saisoku_flag = 0;
    saisoku = (saisoku_flag) && (myf_kosuu / 38);

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            if ((aite_ba[i][j] != 0) && (aite_ba[i][j] < 6)) {
                aite_kosuu_iro++;
            }
            if ((aite_ba[i][j] != 0)) {
                aite_ojama++;
            }
        }
    }
    aite_ojama = aite_ojama - aite_kosuu_iro;
    if (aite_ojama > 5)
        one_tanpatu = 0;
    if ((myf_kosuu_iro >= aite_kosuu_iro * 2) && (cchai > 6) && (aite_hakka_rensa < 4)) {
        kes2 = 1;
    } else {
        kes2 = 0;
    }

    nx1 = tsumo[0];
    nx2 = tsumo[1];
    nn1 = (tsumo[2] == TL_UNKNOWN ? TL_RED : tsumo[2]);
    nn2 = (tsumo[3] == TL_UNKNOWN ? TL_RED : tsumo[3]);
    nk1 = TL_RED;
    nk2 = TL_RED;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 13; j++) {
            ba2[i][j] = ba3[i][j];
        }
    }

    for (dd = 0; dd < 221; dd++) {
        for (aa = 0; aa < 22; aa++) {
            for (bb = 0; bb < 22; bb++) {
                score_hukasa[aa][bb][dd] = 3000;
                for (ee = 0; ee < EE_SIZE; ee++) {
                    chainhyk[aa][bb][dd][ee] = 0;
                    poihyo[aa][bb][dd][ee] = -2000;
                }
            }
        }
    }

    for (aa = 0; aa < 22; aa++) {
        hym[aa] = -10000;
        zenke[aa] = 0;
    }

    kurai_large = 0;
    kurai_middle = 0;
    kurai_small = 0;
    if (aite_hakka_nokori < 3) {
        if ((((aite_hakka_rensa > 3) || (aite_hakka_kosuu > 15)) && (aite_hakka_jamako > 4))
            || ((aite_hakka_zenkesi == 1) && (aite_hakka_jamako > 35))
            || ((aite_hakka_zenkesi == 1) && (aite_hakka_jamako > 35))) {
            kurai_large = 1;
        } else if ((aite_hakka_kosuu > 12) && (aite_hakka_jamako > 4)) {
            kurai_middle = 1;
        } else if ((aite_hakka_kosuu > 8) && (aite_hakka_jamako > 4)) {
            kurai_small = 1;
        } else if ((aite_hakka_jamako > y_t)) {
            kurai_mini = 1;
        }
    }

    wariko_taiou = 0;
    if ((((aite_hakka_rensa > 3) || (aite_hakka_kosuu > 15)) && (aite_hakka_jamako > 4))
        || ((aite_hakka_zenkesi == 1) && (zenkesi_own != 1))) {
        wariko_taiou = 1;
    } else if ((aite_hakka_kosuu > 12) && (aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((aite_hakka_kosuu > 8) && (aite_hakka_jamako > 4)) {
        wariko_taiou = 1;
    } else if ((aite_hakka_jamako > 3)) {
        wariko_taiou = 1;
    }

    if (aite_hakka_rensa < 5) {
        if (myf_kosuu < 15) {
            for (aa = 0; aa < 22; aa++) {
                memcpy(ba_a, ba2, sizeof(ba_a));
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                    chig_aa = 1;
                else
                    chig_aa = 0;
                hon_syoukyo_score(ba_a, &score, &quick);
                if ((ba_a[0][0] == 0) && (ba_a[1][0] == 0) && (ba_a[2][0] == 0) && (ba_a[3][0] == 0)
                    && (ba_a[4][0] == 0) && (ba_a[5][0] == 0)) {
                    for (bb = 0; bb < 22; bb++) {
                        for (dd = 0; dd < 22; dd++) {
                            zenkes[aa][bb][dd] += score + 2100 - chig_aa * 3;
                        }
                    }
                }

                if (myf_kosuu < 13) {
                    for (bb = 0; bb < 22; bb++) {
                        memcpy(ba_ee, ba_a, sizeof(ba_ee));
                        setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                        if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                            chig_bb = 1;
                        else
                            chig_bb = 0;
                        hon_syoukyo_score(ba_ee, &score, &quick);
                        if ((ba_ee[0][0] == 0) && (ba_ee[1][0] == 0) && (ba_ee[2][0] == 0) && (ba_ee[3][0] == 0)
                            && (ba_ee[4][0] == 0) && (ba_ee[5][0] == 0)) {
                            for (dd = 0; dd < 22; dd++) {
                                zenkes[aa][bb][dd] += score + 2100 - (chig_aa + chig_bb) * 3;
                            }
                        }
                    }
                } // p<6
            }
        } // p<7
    }

    for (aa = 0; aa < 22; aa++) {
        for (bb = 0; bb < 22; bb++) {
            for (dd = 0; dd < 22; dd++) {
                if (zenchain <= zenkes[aa][bb][dd]) {
                    zenchain = zenkes[aa][bb][dd];
                    zenchk = aa;
                }
            }
        }
    }
    if (zenchain > 2100)
        zenke[zenchk] += 120000;

    num = 0;

    for (aa = 0; aa < 22; aa++) {

        for (i = 0; i < 20; i++) {
            dabuchk[i] = 0;
        }
        ichiren_kesi = 0;

        if (tobashi_hantei_a(ba2, aa, nx1, nx2))
            continue;

        chain = 0;

        memcpy(ba, ba2, sizeof(ba));

        setti_puyo(ba, aa, nx1, nx2, setti_basyo);
        chain = hon_syoukyo_score(ba, &score, &quick);

        // つぶし
        if ((e_t) && (aite_hakka_rensa < 4)) {
            if ((chain == 2) && (score > aite_rensa_score + 260) && (score > 1600)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }

        if ((hukks > tubushiturn) && (aite_hakka_rensa < 4) && (aite_kosuu_iro > 12)) {
            if ((chain == 2) && (score > aite_rensa_score + 260) && (score > 690)) {
                hym[aa] += 77000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
            if (((chain == 3) || (chain == 4)) && (aite_rensa_score_cc < 3)) {
                hym[aa] += 72000 + score;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }
        if ((maxch <= chain) && (maxscore < score)) {
            maxch = chain;
            maxach = aa;
            maxscore = score;
        }
        myf_kosuu_kesi = 0;
        for (i = 0; i < 6; i++) {
            for (j = 0; j < 13; j++) {
                if ((ba[i][j] != 0) && (ba[i][j] < 6)) {
                    myf_kosuu_kesi++;
                }
            }
        }
        if ((chain == 1) && (score > 690) && (aite_hakka_rensa == 0)) {
            hym[aa] += 74000;
            keschk = 1;
        }
        // さいそく
        if (saisoku == 1) {
            if ((myf_kosuu_iro - myf_kosuu_kesi + 10 < myf_kosuu_kesi) && (score > nocc_aite_rensa_score + 130 * chain)
                && (chain > 1) && (aite_hakka_rensa == 0)) {
                hym[aa] += 73000;
                keschk = 1;
                kougeki_iryoku = score / 70;
                kougeki_on = 1;
            }
        }
        // kurai 対応
        if (u_t == 0) {
            if (((score > aite_hakka_jamako * 70 - 280) && (score < aite_hakka_jamako * 70 + 1400))
                && ((kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1)) && (aite_hakka_honsen == 0)
                && (chain > 1)) {
                hym[aa] += 120000;
                keschk = 1;
            }
        }
        if (zenkesi_own == 1) { // aaaa0909
            if ((chain == 1) && (aite_hakka_zenkesi == 1) && (aite_hakka_jamako < 36)) { // add121014
                hym[aa] += 74000;
            }
            if ((chain > 0) && (zenkesi_aite != 1)) {
                hym[aa] += 74000;
            }
        }
    } // aa

    if (kes2 == 1) {
        if ((maxch > cchai) && (cchai > 1)) {
            hym[maxach] += 150000;
            keschk = 1;
        } else if ((maxch > cchai - 1) && (cchai - 1 > 1)) {
            hym[maxach] += 148000;
            keschk = 1;
        } else if ((maxch > cchai - 2) && (cchai - 2 > 1)) {
            hym[maxach] += 146000;
            keschk = 1;
        }
    }
    if ((wariko_taiou == 1) && (aite_hakka_honsen == 0) && (keschk == 0)) {
        wariko_taiou = 1;
    } else {
        wariko_taiou = 0;
    }

    if (((kurai_mini == 1) || (kurai_small == 1) || (kurai_middle == 1) || (kurai_large == 1))
        && (aite_hakka_honsen == 0) && (keschk == 0)) {
        taiouchk = 1;
    }
    if (((kurai_mini == 1) && (keschk == 0) && (myf_kosuu > 64)) || ((kurai_large == 1) && (keschk == 0))
        || ((kurai_middle == 1) && (keschk == 0)) || ((kurai_small == 1) && (keschk == 0))
        || ((aite_hakka_honsen == 1) && (aite_hakka_nokori < 3))) {
        kuraichk = 1;
    }

    num = 0;
    if ((aite_hakka_honsen == 0) && (q_t)) {
        if (myf_kosuu > 23) {
            for (aa = 0; aa < 22; aa++) {
                if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                    continue;
                memcpy(ba_a, ba2, sizeof(ba));
                setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
                if (chousei_syoukyo(ba_a, setti_basyo) != 0)
                    continue;
                for (cplace = 0; cplace < 6; cplace++) {
                    if (ba_a[0][11] != 0) {
                        if (cplace == 0)
                            continue;
                    }
                    if (ba_a[1][11] != 0) {
                        if ((cplace == 0) || (cplace == 1))
                            continue;
                    }
                    if (ba_a[3][11] != 0) {
                        if ((cplace == 3) || (cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[4][11] != 0) {
                        if ((cplace == 4) || (cplace == 5))
                            continue;
                    }
                    if (ba_a[5][11] != 0) {
                        if (cplace == 5)
                            continue;
                    }
                    for (cyy = 0; cyy < 2; cyy++) {
                        for (ccolor = 1; ccolor < 5; ccolor++) {
                            memcpy(ba, ba_a, sizeof(ba));
                            coita = 0;
                            for (j = 0; j < (12 - cyy); j++) {
                                if (ba[cplace][j + cyy] == 0) {
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
                            chain = 0;
                            chousei_syoukyo_2(ba, setti_basyo, &chain, dabuchk, &ichiren_kesi, &score);
                        }
                    } // cyy
                } // cc
            } // aa
        }

    } // aite_hakka_honsen==0

    if (a_t == 0)
        wariko_taiou = taiouchk;

    cchai = 0;
    tesuu_mon = 0;
    chig_aa = 0;
    chig_bb = 0;
    chig_dd = 0;

    for (dd = 0; dd < 221; dd++) {
        if ((fast != 0) && (dd < 220))
            continue;
        if (dd < 22) {
            nk1 = TL_RED;
            nk2 = TL_RED;
        } else if (dd < 22 * 2) {
            nk1 = TL_RED;
            nk2 = TL_BLUE;
        } else if (dd < 22 * 3) {
            nk1 = TL_RED;
            nk2 = TL_YELLOW;
        } else if (dd < 22 * 4) {
            nk1 = TL_RED;
            nk2 = TL_GREEN;
        } else if (dd < 22 * 5) {
            nk1 = TL_BLUE;
            nk2 = TL_BLUE;
        } else if (dd < 22 * 6) {
            nk1 = TL_BLUE;
            nk2 = TL_YELLOW;
        } else if (dd < 22 * 7) {
            nk1 = TL_BLUE;
            nk2 = TL_GREEN;
        } else if (dd < 22 * 8) {
            nk1 = TL_YELLOW;
            nk2 = TL_YELLOW;
        } else if (dd < 22 * 9) {
            nk1 = TL_YELLOW;
            nk2 = TL_GREEN;
        } else if (dd < 22 * 10) {
            nk1 = TL_GREEN;
            nk2 = TL_GREEN;
        } else {
            nk1 = 0;
            nk2 = 0;
        }

        // Skip some conditions, assuming same colors
        if ((nk1 == 1) && (nk2 == 1)) {
            if (((dd > 5) && (dd < 12)) || ((dd > 13) && (dd < 19)))
                continue;
        }
        if ((nk1 == 2) && (nk2 == 2)) {
            if (((dd > 5 + 88) && (dd < 12 + 88)) || ((dd > 13 + 88) && (dd < 19 + 88)))
                continue;
        }
        if ((nk1 == 3) && (nk2 == 3)) {
            if (((dd > 5 + 154) && (dd < 12 + 154)) || ((dd > 13 + 154) && (dd < 19 + 154)))
                continue;
        }
        if ((nk1 == 4) && (nk2 == 4)) {
            if (((dd > 5 + 198) && (dd < 12 + 198)) || ((dd > 13 + 198) && (dd < 19 + 198)))
                continue;
        }

        for (aa = 0; aa < 22; aa++) {
            if (tobashi_hantei_a(ba2, aa, nx1, nx2))
                continue;
            memcpy(ba_a, ba2, sizeof(ba));
            setti_puyo(ba_a, aa, nx1, nx2, setti_basyo);
            if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                chig_aa = 1;
            else
                chig_aa = 0;
            keshiko_aa = chousei_syoukyo_sc(ba_a, setti_basyo, &score_tmp);
            score_aonly = score_tmp; // only
            score_tmp2 = score_tmp;
            if ((ba_a[2][11] == 0) && (score_tmp > score_aa)) {
                score_aa = score_tmp;
                aa_max_score = aa;
                tesuu_mon = 1;
            }
            if ((ba_a[2][11] == 0) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                && (aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > aite_hakka_jamako * 70)) {
                if ((myf_kosuu_iro - 1 > keshiko_aa * 2) || (i_t)) {
                    score_tai = score_tmp2;
                }
            }
            if ((myf_kosuu_iro - keshiko_aa + 8) < cchai * 4)
                continue;
            if (ba_a[2][11] != 0) {
                for (bb = 0; bb < 22; bb++) {
                    for (ee = 0; ee < EE_SIZE; ee++) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -3000;
                    }
                }
                continue;
            } // 110604

            for (bb = 0; bb < 22; bb++) {
                if (tobashi_hantei_a(ba_a, bb, nn1, nn2))
                    continue;
                memcpy(ba_ee, ba_a, sizeof(ba));
                setti_puyo(ba_ee, bb, nn1, nn2, setti_basyo);
                if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                    chig_bb = 1;
                else
                    chig_bb = 0;
                keshiko_bb = chousei_syoukyo_sc(ba_ee, setti_basyo, &score_tmp);
                score_bonly = score_tmp; // only
                score_tmp2 = score_tmp;
                if ((kuraichk == 1) && (aite_hakka_nokori < 2) && (score_aonly > 0))
                    score_tmp = 0; // only
                if ((kuraichk == 1)
                    && ((aite_hakka_nokori < 1)
                        || ((aite_hakka_nokori < 2) && ((aite_puyo_uki == 0) && (aite_hakka_quick == 1)))))
                    score_tmp = 0; // 0225
                if ((ba_ee[2][11] == 0) && (score_tmp > score_aa)) {
                    score_aa = score_tmp;
                    aa_max_score = aa;
                    tesuu_mon = 2;
                }
                if ((wariko_taiou == 1) && (aite_hakka_nokori < 2) && (score_aonly > 0))
                    score_tmp2 = 0; // only
                if ((wariko_taiou == 1)
                    && ((aite_hakka_nokori < 1)
                        || ((aite_hakka_nokori < 2) && ((aite_puyo_uki == 0) && (aite_hakka_quick == 1)))))
                    score_tmp2 = 0; // 0225
                if ((ba_ee[2][11] == 0) && (score_tmp2 > score_tai) && (score_tmp2 > 270)
                    && (aite_hakka_jamako * 70 > score_tmp2 - 1400) && (score_tmp2 + 150 > aite_hakka_jamako * 70)) {
                    if ((myf_kosuu_iro + 1 > (keshiko_aa + keshiko_bb) * 2) || (i_t)) {
                        score_tai = score_tmp2;
                    }
                }
                if ((myf_kosuu_iro - keshiko_aa - keshiko_bb + 8) < cchai * 4)
                    continue;
                if (ba_ee[2][11] != 0) {
                    for (ee = 0; ee < EE_SIZE; ee++) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -3000;
                    }
                    continue;
                } // 110604

                // NOTE: In case dd==220, we assume the 3rd Tsumo is (EMPTY,EMPTY).
                if (dd < 220) {
                    if (tobashi_hantei_b(ba_ee, dd % 22))
                        continue;
                    setti_puyo(ba_ee, dd % 22, nk1, nk2, setti_basyo);
                    if ((setti_basyo[0] != setti_basyo[2]) && (setti_basyo[1] != setti_basyo[3]))
                        chig_dd = 1;
                    else
                        chig_dd = 0;
                    keshiko_dd = chousei_syoukyo_sc(ba_ee, setti_basyo, &score_tmp);
                    score_tmp2 = score_tmp;
                    if ((kuraichk == 1) && ((score_aonly > 0) || (score_bonly > 0)))
                        score_tmp = 0; // only
                    if ((kuraichk == 1) && (aite_hakka_nokori < 3))
                        score_tmp = 0;

                    if (myf_kosuu < 56) {
                        if ((ba_ee[2][11] == 0) && (score_tmp > score_aa)) {
                            score_aa = score_tmp;
                            aa_max_score = aa;
                            tesuu_mon = 3;
                        }
                    } else if (myf_kosuu < 62) {
                        if ((ba_ee[2][11] == 0) && (score_tmp * 6 / 7 > score_aa)) {
                            score_aa = score_tmp * 6 / 7;
                            aa_max_score = aa;
                            tesuu_mon = 3;
                        }
                    } else if (myf_kosuu < 64) {
                        if ((ba_ee[2][11] == 0) && (score_tmp * 3 / 4 > score_aa)) {
                            score_aa = score_tmp * 3 / 4;
                            aa_max_score = aa;
                            tesuu_mon = 3;
                        }
                    } else {
                        if ((ba_ee[2][11] == 0) && (score_tmp * 1 / 2 > score_aa)) {
                            score_aa = score_tmp * 1 / 2;
                            aa_max_score = aa;
                            tesuu_mon = 3;
                        }
                    }
                    if ((myf_kosuu_iro - keshiko_aa - keshiko_bb - keshiko_dd + 8) < cchai * 4)
                        continue;
                    if (ba_ee[2][11] != 0) {
                        for (ee = 0; ee < EE_SIZE; ee++) {
                            chainhyk[aa][bb][dd][ee] = 0;
                            poihyo[aa][bb][dd][ee] = -3000;
                        }
                        continue;
                    } // 110604
                } // dd220

                // Penalty for CHIGIRI
                if (aite_hakka_honsen == 0) {
                    if (p_t == 4)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 3 + chig_bb * 2 + chig_dd * 1) * 400;
                    if (p_t == 3)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 3 + chig_bb * 2 + chig_dd * 1) * 30;
                    if (p_t == 2)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (p_t == 1)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 1 + chig_bb * 1 + chig_dd * 1);
                } else {
                    if (p_t == 4)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (p_t == 3)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (p_t == 2)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 2 + chig_bb * 1 + chig_dd * 1);
                    if (p_t == 1)
                        score_hukasa[aa][bb][dd] -= (chig_aa * 1 + chig_bb * 1 + chig_dd * 1);
                }

                // Penalty for field like 'U'
                if (ba_ee[2][10] != 0)
                    score_hukasa[aa][bb][dd] -= 1200;
                if (ba_ee[3][10] != 0)
                    score_hukasa[aa][bb][dd] -= 1200;
                for (j = 0; j < 9; j++) {
                    if (ba_ee[0][j] == 0) {
                        if (ba_ee[1][j + 1] != 0)
                            score_hukasa[aa][bb][dd] -= 300;
                        break;
                    }
                }
                for (i = 1; i < 5; i++) {
                    for (j = 0; j < 9; j++) {
                        if (ba_ee[i][j] == 0) {
                            if ((ba_ee[i - 1][j + 1] != 0) && (ba_ee[i + 1][j + 1] != 0))
                                score_hukasa[aa][bb][dd] -= 300;
                            break;
                        }
                    }
                }
                for (j = 0; j < 9; j++) {
                    if (ba_ee[5][j] == 0) {
                        if (ba_ee[4][j + 1] != 0)
                            score_hukasa[aa][bb][dd] -= 300;
                        break;
                    }
                }

                // GTR-check
                if ((zenkesi_own != 1) && (zenkesi_aite != 1) && (o_t)) {
                    if (hukks < 10)
                        score_hukasa[aa][bb][dd] += gtr(ba_ee);
                }

                // Balanced field
                for (i = 0; i < 6; i++) {
                    for (j = 0; j < 14; j++) {
                        if (ba_ee[i][j] == 0) {
                            teimen[i] = j;
                            break;
                        }
                    }
                }
                if ((zenkesi_aite != 1) && (r_t)) {
                    aveteimen = teimen[0] + teimen[1] + teimen[2] + teimen[3] + teimen[4] + teimen[5];
                    teimen[0] = (teimen[0] - 3) * 6 - aveteimen;
                    if (teimen[0] < 0)
                        teimen[0] = teimen[0] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[0] * 10;
                    teimen[1] = (teimen[1]) * 6 - aveteimen;
                    if (teimen[1] < 0)
                        teimen[1] = teimen[1] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[1] * 10;
                    teimen[2] = (teimen[2] + 1) * 6 - aveteimen;
                    if (teimen[2] < 0)
                        teimen[2] = teimen[2] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[2] * 10;
                    teimen[3] = (teimen[3] + 1) * 6 - aveteimen;
                    if (teimen[3] < 0)
                        teimen[3] = teimen[3] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[3] * 10;
                    teimen[4] = (teimen[4]) * 6 - aveteimen;
                    if (teimen[4] < 0)
                        teimen[4] = teimen[4] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[4] * 10;
                    teimen[5] = (teimen[5] - 3) * 6 - aveteimen;
                    if (teimen[5] < 0)
                        teimen[5] = teimen[5] * (-1);
                    score_hukasa[aa][bb][dd] -= teimen[5] * 10;
                }

                // Tanpatsu
                tanpatu_on = keshiko_aa * 2 + keshiko_bb;
                if (keshiko_aa < 5 && keshiko_bb < 5)
                    tanpatu_on = 0;
                if (keshiko_aa > 7 || keshiko_bb > 7)
                    tanpatu_on = 0;
                if (tanpatu_on > 0 && myf_kosuu_iro > aite_kosuu_iro + 1 && myf_kosuu_iro > 23 && one_tanpatu && aite_hakka_kosuu == 0 && aite_hakka_rensa == 0)
                    score_hukasa[aa][bb][dd] += tanpatu_on * 120;

                // TODO(peria): what ee means?
                for (ee = 0; ee < EE_SIZE; ee++) {
                    if ((key_ee == 0) && (ee > 0))
                        break; // t2
                    if (ee == 0) {
                        eex = 0;
                        eecol = 0;
                    } else {
                        eex = (ee - 1) % 6;
                        eecol = (ee - 1) / 6 + 1;
                        if ((eex == 2) || (eex == 3))
                            continue;
                        if (ba_ee[0][11] != 0) {
                            if (eex == 0)
                                continue;
                        }
                        if (ba_ee[1][11] != 0) {
                            if ((eex == 0) || (eex == 1))
                                continue;
                        }
                        if (ba_ee[3][11] != 0) {
                            if ((eex == 4) || (eex == 5))
                                continue;
                        }
                        if (ba_ee[4][11] != 0) {
                            if ((eex == 4) || (eex == 5))
                                continue;
                        }
                        if (ba_ee[5][11] != 0) {
                            if (eex == 5)
                                continue;
                        }
                    }
                    memcpy(ba, ba_ee, sizeof(ba));
                    if (ee != 0) {
                        if (setti_puyo_1(ba, eex, eecol))
                            continue;
                    }

                    if (ba[2][11] != 0) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -3000;
                        continue;
                    }
                    if ((ba[0][10] == 0) && (ba[1][11] != 0) && (ba2[1][11] == 0)) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }
                    if (((ba[5][10] == 0) || (ba[4][10] == 0)) && (ba[3][11] != 0) && (ba2[3][11] == 0)) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }
                    if ((ba[5][10] == 0) && (ba[4][11] != 0) && (ba2[4][11] == 0)) {
                        chainhyk[aa][bb][dd][ee] = 0;
                        poihyo[aa][bb][dd][ee] = -2000;
                        continue;
                    }
                    for (i = 0; i < 6; i++) {
                        for (j = 0; j < 12; j++) {
                            point2[i][j] = 8;
                            if (ba[i][j] != 0 && ba[i][j] != 6) {
                                if (i != 5 && ba[i][j + 1] == 0 && ba[i + 1][j] == 0) {
                                    if (j != 11 && (i != 4 || ba[3][11] == 0)) {
                                        point2[i][j] = 2;
                                        point2[i][j + 1] = 9;
                                        break;
                                    }
                                }
                                if (i != 0 && ba[i][j + 1] == 0 && ba[i - 1][j] == 0) {
                                    if (j != 11 && (i != 5 || ba[3][11] == 0)) {
                                        point2[i][j] = 3;
                                        point2[i][j + 1] = 9;
                                        break;
                                    }
                                }
                                if (ba[i][j + 1] == 0) {
                                    if (j != 11 &&  (i != 0 || ba[1][11] == 0) && (i != 4 || ba[3][11] == 0)
                                        && (i != 5 || (ba[3][11] == 0 && ba[4][11] == 0))) {
                                        point2[i][j] = 4;
                                        point2[i][j + 1] = 9;
                                        break;
                                    }
                                }
                                if ((i !=  5 && ba[i + 1][j] == 0) && (i != 0 && ba[i - 1][j] == 0)) {
                                    if (ba[i][11] == 0) {
                                        point2[i][j] = 5;
                                        continue;
                                    }
                                }
                                if (i != 5 && ba[i + 1][j] == 0) {
                                    if (((i != 4) || ((ba[3][11] == 0) && (ba[4][11] == 0)))
                                        && ((i != 3) || (ba[3][11] == 0))) {
                                        point2[i][j] = 6;
                                        continue;
                                    }
                                }
                                if (i != 0 && ba[i - 1][j] == 0) {
                                    if (((i != 1) || (ba[1][11] == 0)) && ((i != 5) || (ba[3][11] == 0))) {
                                        point2[i][j] = 7;
                                        continue;
                                    }
                                }
                            }
                        }
                    }

                    num2 = 3;
                    for (i2 = 0; i2 < 6; i2++) {
                        hakkatakasa = 0;
                        for (j2 = 0; j2 < 12; j2++) {
                            if (point2[i2][j2] == 9)
                                break;
                            if (point2[i2][j2] == 8)
                                continue;
                            if (hakkatakasa < 6)
                                hakkatakasa++;
                            if (point2[i2][j2] == 1)
                                continue;
                            if ((num2 > 2))
                                memcpy(bass, ba, sizeof(bass));
                            chain = 0;
                            num2 = 0;
                            poi2s = 0;
                            tokus = point2[i2][j2];
                            saiki_3(bass, point2, i2, j2, &num2, bass[i2][j2]);
                            if ((num2 < 3))
                                goto POSS;
                            poi2s = j2 * takasa_point;
                            if (j2 > 5)
                                poi2s += takasa_point;
                            if ((tokus > 1) && (tokus < 5))
                                poi2s += 100 * t_t;
                            if (zenkesi_aite == 1)
                                poi2s = hakkatakasa * 300;

                            chain = chousei_syoukyo_3(bass, setti_basyo, &poi2s, &score_mm, tokus, i2, j2);

                            if ((dd < 220) && (myf_kosuu > 63))
                                score_mm = score_mm * 1 / 2;
                            else if ((dd < 220) && (myf_kosuu > 61))
                                score_mm = score_mm * 3 / 4;
                            else if ((dd < 220) && (myf_kosuu > 55))
                                score_mm = score_mm * 6 / 7;

                        POSS:
                            pois = 0;
                            if (cchai <= chain) {
                                cchai = chain;
                                memset(point, 0, sizeof(point));
                                num = 0;
                                for (i = 0; i < 6; i++) {
                                    for (j = 0; j < 12; j++) {
                                        if (bass[i][j] == 0)
                                            break;
                                        if ((point[i][j] != 1) && (bass[i][j] != 6)) {
                                            saiki(bass, point, i, j, &num, bass[i][j]);
                                            pois = pois + num * num * num;
                                            num = 0;
                                        }
                                    }
                                }
                            }
                            if (chain > cchai - 3) {
                                for (i = 0; i < 5; i++) {
                                    for (j = 0; j < yokotate; j++) {
                                        if ((bass[i][j] != 0) && (bass[i][j] != 6)) {
                                            if (bass[i][j] == bass[i + 1][j])
                                                pois += yokopoint;
                                        }
                                    }
                                } // 12-2
                            }
                            if (chainhyk[aa][bb][dd][ee] < chain) {
                                chainhyk[aa][bb][dd][ee] = chain;
                                poihyo[aa][bb][dd][ee] = (pois + poi2s);
                            } else if (chainhyk[aa][bb][dd][ee] == chain) {
                                if (poihyo[aa][bb][dd][ee] < (pois + poi2s)) {
                                    poihyo[aa][bb][dd][ee] = (pois + poi2s);
                                }
                            }
                            if (kuraichk == 1)
                                score_mm = 0;
                            if (score_mm > score_max) {
                                score_max = score_mm;
                                mmmax = aa;
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
