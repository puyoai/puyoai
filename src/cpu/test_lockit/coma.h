#ifndef CPU_TEST_LOCKIT_COMA_H_
#define CPU_TEST_LOCKIT_COMA_H_

#include "core/puyo_color.h"
#include "cpu_configuration.h"
#include "lockit_constant.h"

class CoreField;

namespace test_lockit {

const int tubushiturn = 13;
// int tubushiturn = 99999;
const int saisoku_point = 1;
// int saisoku_point = 0;

class COMAI_HI {
public:
    explicit COMAI_HI(const cpu::Configuration& config);
    ~COMAI_HI();

    int aite_hyouka(const CoreField& field, PuyoColor tsumo[]);
    int pre_hyouka(const PuyoColor ba3[6][kHeight], PuyoColor tsumo[], int zenkesi_own, PuyoColor aite_ba[6][kHeight], int zenkesi_aite, int fast);
    int hyouka(const PuyoColor ba3[6][kHeight], PuyoColor tsumo[], int zenkesi_own, PuyoColor aite_ba[6][kHeight], int zenkesi_aite);
    bool aite_attack_start(CoreField field, int zenkesi_aite, int scos, int hakata);
    int aite_attack_nokori(const PuyoColor ba3[6][kHeight], int hakata);
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
    int tobashi_hantei_a(const PuyoColor[][kHeight], int, PuyoColor, PuyoColor);
    int tobashi_hantei_b(const PuyoColor[][kHeight], int);

    const cpu::Configuration config;

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

    int m_one_tanpatu;
    int m_score_max;
    int m_mmmax;
    int m_score_aa;
    int m_aa_max_score;
    int m_hakkatime;
};

}  // namespace test_lockit

#endif
