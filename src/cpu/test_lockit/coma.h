#ifndef CPU_TEST_LOCKIT_COMA_H_
#define CPU_TEST_LOCKIT_COMA_H_

#include "lockit_constant.h"

namespace test_lockit {

/* for TAIOU TYPE?
int q_t=1;
int w_t=1;
int e_t=0;
int r_t=1;
int t_t=1;
int y_t=3;
int u_t=1;
int i_t=0;
int o_t=0;
int p_t=3;
int a_t=1;*/

const int tubushiturn = 13;
// int tubushiturn = 99999;
const int saisoku_point = 1;
// int saisoku_point = 0;

class COMAI_HI {
public:
    COMAI_HI();
    ~COMAI_HI();

    int aite_hyouka(const int ba3[6][kHeight], int tsumo[]);
    int pre_hyouka(const int ba3[6][kHeight], int tsumo[], int zenkesi_own, int aite_ba[6][kHeight], int zenkesi_aite, int fast);
    int hyouka(const int ba3[6][kHeight], int tsumo[], int zenkesi_own, int aite_ba[6][kHeight], int zenkesi_aite);
    bool aite_attack_start(const int ba3[6][kHeight], int zenkesi_aite, int scos, int hakata);
    int aite_attack_nokori(const int ba3[6][kHeight], int hakata);
    int aite_rensa_end();
    void ref();

    // Index of TSUMOs in the game.
    int m_hukks;

    // ???
    int m_para[22];

    // Enemy's running rensa will end in |aite_hakka_nokori| rensas.
    // NOTE: Estimation of this value seems wrong.
    int m_aite_hakka_nokori;

    // ???
    int m_aite_hakkaji_score;

private:
    int saiki_3(const int[][kHeight], int[][12], int, int, int*, int);
    int saiki_3_right(const int[][kHeight], int[][12], int, int, int*, int);
    int saiki_3_left(const int[][kHeight], int[][12], int, int, int*, int);
    int saiki_3_up(const int[][kHeight], int[][12], int, int, int*, int);
    int saiki_3_down(const int[][kHeight], int[][12], int, int, int*, int);

    int saiki_4(int[][kHeight], int, int, int*, int);

    int syou(int[][kHeight], int, int, int, int[]);
    int syou_right(int[][kHeight], int, int, int, int[]);
    int syou_left(int[][kHeight], int, int, int, int[]);
    int syou_up(int[][kHeight], int, int, int, int[]);
    int syou_down(int[][kHeight], int, int, int, int[]);
    int syou_downx(int[][kHeight], int, int, int, int[], int*);
    int syou_right_num(int[][kHeight], int, int, int, int[], int*);
    int syou_left_num(int[][kHeight], int, int, int, int[], int*);
    int syou_up_num(int[][kHeight], int, int, int, int[], int*);
    int syou_down_num(int[][kHeight], int, int, int, int[], int*);
    int syou_2(int[][kHeight], int, int, int);

    int setti_ojama(int[][kHeight], int);

    int tobashi_hantei_a(const int[][kHeight], int, int, int);
    bool setti_puyo(int[][kHeight], int, int, int, int[]);
    int tobashi_hantei_b(const int[][kHeight], int);
    int chousei_syoukyo(int[][kHeight], int[]);
    int chousei_syoukyo_2(int[][kHeight], int[], int*, int[], int*, int*);
    int chousei_syoukyo_3(int[][kHeight], int[], int*, int*, int, int, int);
    int chousei_syoukyo_sc(int[][kHeight], int[], int*);
    int hon_syoukyo(int[][kHeight]);
    int hon_syoukyo_score(int[][kHeight], int*, int*);

    int setti_puyo_1(int[][kHeight], int, int);
    int gtr(const int[][kHeight]);

    int read_template();

    int m_cchai;
    int m_conaa;
    int m_nexaa;
    int m_maxchais;
    int m_aite_rensa_score; // aite
    int m_aite_rensa_score_cc; // aite
    int m_myf_kosuu;
    int m_saisoku_flag;
    int m_aite_hakka_rensa;
    int m_aite_hakka_zenkesi;
    int m_aite_hakka_kosuu;
    int m_nocc_aite_rensa_score;
    int m_max_ee;
    int m_key_ee;

    int m_aite_hakka_jamako;
    int m_aite_hakka_honsen;
    int m_aite_hakka_quick;
    int m_moni_kesiko[3];
    int m_moni_iroko[3];
    int m_aite_puyo_uki;
    int m_kuraichk_mon;
    int m_score_mon;
    int m_tesuu_mon;
    int m_kougeki_on;
    int m_kougeki_edge;
    int m_kougeki_iryoku;

    int m_numg;
    int m_katax[TM_TMNMUM][TM_COLNUM][TM_COLPER];
    int m_katay[TM_TMNMUM][TM_COLNUM][TM_COLPER];
    int m_kko[TM_TMNMUM][TM_COLNUM];
    int m_colko[TM_TMNMUM];
    int m_colscore[TM_TMNMUM][TM_COLNUM];
    int m_kinko[TM_TMNMUM];
    int m_kinsi_a[TM_TMNMUM][TM_KINPT];
    int m_kinsi_b[TM_TMNMUM][TM_KINPT];
    int m_jakkinko[TM_TMNMUM];
    int m_jakkin_a[TM_TMNMUM][TM_JAKKINPT];
    int m_jakkin_b[TM_TMNMUM][TM_JAKKINPT];
    int m_tankinx[TM_TMNMUM][TM_TANKINPT];
    int m_tankiny[TM_TMNMUM][TM_TANKINPT];
    int m_tankinko[TM_TMNMUM];
    int m_jatax[TM_TMNMUM][TM_OBJE];
    int m_jatay[TM_TMNMUM][TM_OBJE];
    int m_jatako[TM_TMNMUM];
    int m_tm_turn[TM_TMNMUM];
    int m_tm_moni;
    int m_one_tanpatu;
    int m_score_max;
    int m_mmmax;
    int m_score_aa;
    int m_aa_max_score;
    int m_hakkatime;
};

}  // namespace test_lockit

#endif
