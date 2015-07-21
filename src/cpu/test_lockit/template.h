#ifndef CPU_TEST_LOCKIT_TEMPLATE_H_
#define CPU_TEST_LOCKIT_TEMPLATE_H_

#include <string>

#include "color.h"
#include "lockit_constant.h"

namespace test_lockit {

int gtr(const TLColor f[][kHeight]);

class Template {
public:
    explicit Template(const std::string& filename);

    // The number of template patterns.
    int m_numg;

    int m_kko[TM_TMNMUM][TM_COLNUM];
    int m_colko[TM_TMNMUM];
    int m_katax[TM_TMNMUM][TM_COLNUM][TM_COLPER];
    int m_katay[TM_TMNMUM][TM_COLNUM][TM_COLPER];
    int m_colscore[TM_TMNMUM][TM_COLNUM];
    int m_kinko[TM_TMNMUM];
    int m_jakkinko[TM_TMNMUM];
    int m_tankinx[TM_TMNMUM][TM_TANKINPT];
    int m_tankiny[TM_TMNMUM][TM_TANKINPT];
    int m_tankinko[TM_TMNMUM];
    int m_jatax[TM_TMNMUM][TM_OBJE];
    int m_jatay[TM_TMNMUM][TM_OBJE];
    int m_jatako[TM_TMNMUM];
    int m_kinsi_a[TM_TMNMUM][TM_KINPT];
    int m_kinsi_b[TM_TMNMUM][TM_KINPT];
    int m_jakkin_a[TM_TMNMUM][TM_JAKKINPT];
    int m_jakkin_b[TM_TMNMUM][TM_JAKKINPT];
    int m_tm_turn[TM_TMNMUM];
};

}  // namespace test_lockit

#endif  // CPU_TEST_LOCKIT_TEMPLATE_H_
