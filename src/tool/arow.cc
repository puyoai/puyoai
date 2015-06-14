#include <algorithm>
#include <vector>

#include <dirent.h>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <SDL_image.h>

#include "base/strings.h"
#include "capture/color.h"
#include "core/real_color.h"
#include "gui/unique_sdl_surface.h"
#include "gui/util.h"

DECLARE_string(testdata_dir);

using namespace std;

class Arow {
public:
    Arow() : SIZE(16 * 16 * 3), RATE(0.1), mean(SIZE), cov(SIZE)
    {
        std::fill(cov.begin(), cov.end(), 1.0);
    }

    double margin(const vector<double>& features) const
    {
        double result = 0.0;
        for (int i = 0; i < SIZE; ++i) {
            result += mean[i] * features[i];
        }
        return result;
    }

    double confidence(const vector<double>& features) const
    {
        double result = 0.0;
        for (int i = 0; i < SIZE; ++i) {
            result += cov[i] * features[i] * features[i];
        }
        return result;
    }

    int update(const vector<double>& features, int label)
    {
        double m = margin(features);
        int loss = m * label < 0 ? 1 : 0;
        if (m * label >= 1)
            return 0;

        double v = confidence(features);
        double beta = 1.0 / (v + RATE);
        double alpha = (1.0 - label * m) * beta;

        // update mean
        for (int i = 0; i < SIZE; ++i) {
            mean[i] += alpha * label * cov[i] * features[i];
        }

        // update covariance
        for (int i = 0; i < SIZE; ++i) {
            cov[i] = 1.0 / ((1.0 / cov[i]) + features[i] * features[i] / RATE);
        }

        return loss;
    }

    int predict(const vector<double>& features) const
    {
        double m = margin(features);
        return m > 0 ? 1 : -1;
    }

    void save(const char* filename) const
    {
        FILE* fp = fopen(filename, "wb");
        PCHECK(fp);

        fwrite(&mean[0], sizeof(double), SIZE, fp);
        fwrite(&cov[0], sizeof(double), SIZE, fp);

        fclose(fp);
    }

private:
    const int SIZE;
    const double RATE;
    vector<double> mean;
    vector<double> cov;
};

enum class RecognitionColor {
    RED,
    BLUE,
    YELLOW,
    GREEN,
    PURPLE,
    EMPTY,
    OJAMA,
    ZENKESHI
};
const int NUM_RECOGNITION = 8;

// ----------------------------------------------------------------------

int main()
{
    const int WIDTH = 16;
    const int HEIGHT = 16;

    Arow arows[NUM_RECOGNITION];
    vector<vector<double>> features[NUM_RECOGNITION];

    const pair<string, RecognitionColor> testcases[] = {
        make_pair((FLAGS_testdata_dir + "/images/recognition/R"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/recognition/B"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/recognition/G"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/recognition/Y"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/recognition/P"), RecognitionColor::PURPLE),
        make_pair((FLAGS_testdata_dir + "/images/recognition/E"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/recognition/O"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/recognition/Z"), RecognitionColor::ZENKESHI),
    };

    // Read training data (2)
    for (const auto& testcase : testcases) {
        string dirname = testcase.first;
        RecognitionColor color = testcase.second;

        DIR* dir = opendir(dirname.c_str());
        PCHECK(dir) << dirname.c_str();

        struct dirent* ent;
        while ((ent = readdir(dir)) != nullptr) {
            string path = dirname + "/" + ent->d_name;
            if (!strings::isSuffix(path, ".bmp"))
                continue;

            UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(path.c_str())));
            CHECK(surf.get()) << path.c_str();

            int pos = 0;
            vector<double> fs(WIDTH * HEIGHT * 3);
            for (int x = 0; x < WIDTH; ++x) {
                for (int y = 0; y < HEIGHT; ++y) {
                    std::uint32_t c = getpixel(surf.get(), x, y);
                    std::uint8_t r, g, b;
                    SDL_GetRGB(c, surf->format, &r, &g, &b);
                    fs[pos++] = r;
                    fs[pos++] = g;
                    fs[pos++] = b;
                }
            }

            CHECK_EQ(pos, WIDTH * HEIGHT * 3);

            features[static_cast<int>(color)].push_back(std::move(fs));
        }
        closedir(dir);
    }

    const pair<string, RecognitionColor> training_testcases[] = {
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty.png"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/red.png"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue.png"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow.png"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green.png"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama.png"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple.png"), RecognitionColor::PURPLE),

        make_pair((FLAGS_testdata_dir + "/images/puyo/empty-blur.png"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/red-blur.png"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue-blur.png"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow-blur.png"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green-blur.png"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama-blur.png"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple-blur.png"), RecognitionColor::PURPLE),
    };

    // Read training testcases.
    for (const auto& testcase: training_testcases) {
        const string& filename = testcase.first;
        const RecognitionColor color = testcase.second;

        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));

        for (int x = 0; (x + 1) * WIDTH <= surf->w; ++x) {
            for (int y = 0; (y + 1) * HEIGHT <= surf->h; ++y) {
                int pos = 0;
                vector<double> fs(WIDTH * HEIGHT * 3);
                for (int xx = 0; xx < WIDTH; ++xx) {
                    for (int yy = 0; yy < HEIGHT; ++yy) {
                        std::uint32_t c = getpixel(surf.get(), x * WIDTH + xx, y * HEIGHT + yy);
                        std::uint8_t r, g, b;
                        SDL_GetRGB(c, surf->format, &r, &g, &b);
                        fs[pos++] = r;
                        fs[pos++] = g;
                        fs[pos++] = b;
                    }
                }

                CHECK(pos == WIDTH * HEIGHT * 3);
                if (color == RecognitionColor::EMPTY) {
                    if (x == 6) {
                        features[static_cast<int>(RecognitionColor::EMPTY)].push_back(std::move(fs));
                    } else {
                        features[static_cast<int>(RecognitionColor::ZENKESHI)].push_back(std::move(fs));
                    }
                } else {
                    features[static_cast<int>(color)].push_back(std::move(fs));
                }
            }
        }
    }

    for (const auto& f : features) {
        cout << "SIZE = " << f.size() << endl;
    }

    // training
    for (int times = 0; times < 200; ++times) {
        for (int i = 0; i < NUM_RECOGNITION; ++i) {
            for (int j = 0; j < static_cast<int>(features[i].size()); ++j) {
                if ((j & 0xF) == 0)
                    continue;

                for (int k = 0; k < NUM_RECOGNITION; ++k) {
                    arows[k].update(features[i][j], i == k ? 1 : -1);
                }
            }
        }

        cout << "training " << times << ": done" << endl;
    }

    // test by all
    int num = 0;
    int fail = 0;
    for (int i = 0; i < NUM_RECOGNITION; ++i) {
        for (int j = 0; j < static_cast<int>(features[i].size()); ++j) {
            ++num;
            double vs[NUM_RECOGNITION] {};
            for (int k = 0; k < NUM_RECOGNITION; ++k) {
                vs[k] = arows[k].margin(features[i][j]);
            }

            int result = std::max_element(vs, vs + NUM_RECOGNITION) - vs;
            if (i != result) {
                cout << "fail: " << i << " " << j << " -> " << result << endl;
                for (int k = 0; k < NUM_RECOGNITION; ++k)
                    cout << vs[k] << ' ';
                cout << endl;
                ++fail;
            }
        }
    }

    cout << "num = " << num << endl;
    cout << "fail = " << fail << endl;

    arows[0].save("red.arow");
    arows[1].save("blue.arow");
    arows[2].save("yellow.arow");
    arows[3].save("green.arow");
    arows[4].save("purple.arow");
    arows[5].save("empty.arow");
    arows[6].save("ojama.arow");
    arows[7].save("zenkeshi.arow");

    return 0;
}
