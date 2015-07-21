#include "template.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "color.h"

namespace test_lockit {

int gtr(const TLColor field[][kHeight])
{
    int score = 0;

    static const struct {
        int x0, y0, x1, y1;
    } expect_same[] = {
        {0, 0, 1, 0}, {0, 0, 1, 2}, {0, 0, 2, 1},
        {1, 0, 1, 2}, {1, 0, 2, 1}, {1, 2, 2, 1},
        {0, 1, 0, 2}, {0, 1, 1, 1}, {1, 1, 0, 2},
    };
    for (const auto& same : expect_same) {
        TLColor c0 = field[same.x0][same.y0];
        TLColor c1 = field[same.x1][same.y1];
        if (c0 == TLColor::EMPTY || c1 == TLColor::EMPTY)
            continue;
        if (c0 == c1)
            score += 1000;
        else
            score -= 1000;
    }

    static const struct {
        int x0, y0, x1, y1;
        int penalty;
    } expect_diff[] = {
        {0, 2, 0, 3, 2000},
        {1, 2, 1, 3, 500}, {1, 2, 2, 2, 500}, {2, 1, 2, 2, 500},
        {1, 0, 2, 0, 1000}, {2, 1, 2, 0, 1000},
    };
    for (const auto& diff : expect_diff) {
        TLColor c0 = field[diff.x0][diff.y0];
        TLColor c1 = field[diff.x1][diff.y1];
        if (c0 != TLColor::EMPTY && c0 == c1)
            score -= diff.penalty;
    }

    return score;
}

Template::Template(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (ifs.fail())
        return;

    int a[78];
    int i = 0;
    int cnt = 0;
    for (std::string line; getline(ifs, line);) {
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream iss(line);
        if (cnt == 0) {
            if (!(iss >> m_colko[i]))
                break;
            ++m_colko[i];
            if (m_colko[i] > TM_COLNUM)
                break;
        } else if (cnt == 1) {
            for (int j = 1; j < m_colko[i]; j++) {
                if (!(iss >> m_colscore[i][j]))
                    break;
            }
        } else if (cnt >= 2 && cnt <= 7) {
            for (int j = (cnt - 2) * 13; j < (cnt - 1) * 13; ++j)
                if (!(iss >> a[j]))
                    break;

            if (cnt == 7) {
                for (int j = 0; j < 78; j++) {
                    if (a[j] > 0 && a[j] < TM_COLNUM) {
                        int aj = a[j];
                        m_katax[i][aj][m_kko[i][aj]] = j / 13;
                        m_katay[i][aj][m_kko[i][aj]] = j % 13;
                        m_kko[i][aj]++;
                    } else if (a[j] == -1) {
                        m_tankinx[i][m_tankinko[i]] = j / 13;
                        m_tankiny[i][m_tankinko[i]] = j % 13;
                        m_tankinko[i]++;
                    } else if (a[j] > 100) {
                        m_jatax[i][a[j] - 101] = j / 13;
                        m_jatay[i][a[j] - 101] = j % 13;
                        m_jatako[i]++;
                    }
                }
            }
        } else if (cnt == 8) {
            if (!(iss >> m_kinko[i]))
                break;
            if (m_kinko[i] > TM_KINPT)
                break;
        } else if (cnt == 9) {
            for (int j = 0; j < m_kinko[i]; j++) {
                if (!(iss >> m_kinsi_a[i][j]))
                    break;
                if (!(iss >> m_kinsi_b[i][j]))
                    break;
            }
        } else if (cnt == 10) {
            if (!(iss >> m_jakkinko[i]))
                break;
            if (m_jakkinko[i] > TM_JAKKINPT)
                break;
        } else if (cnt == 11) {
            for (int j = 0; j < m_jakkinko[i]; j++) {
                if (!(iss >> m_jakkin_a[i][j]))
                    break;
                if (!(iss >> m_jakkin_b[i][j]))
                    break;
            }
        } else if (cnt == 12) {
            if (!(iss >> m_tm_turn[i]))
                break;
        } else if (cnt == 13) {
            if (line.substr(0, 3) != "###")
                break;

            i++;
            cnt = -1;
        }
        cnt++;
    }
    m_numg = i;
}


}  // namespace test_lockit
