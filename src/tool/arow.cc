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
    Arow() : SIZE(32 * 32 * 3), RATE(0.1), mean(SIZE), cov(SIZE)
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

private:
    const int SIZE;
    const double RATE;
    vector<double> mean;
    vector<double> cov;
};

// ----------------------------------------------------------------------

#if defined(__CYGWIN__) && defined(main)
#undef main
#endif

int main()
{
    const int WIDTH = 32;
    const int HEIGHT = 32;

    const pair<string, RealColor> training_testcases[] = {
        make_pair((FLAGS_testdata_dir + "/image/empty.png"), RealColor::RC_EMPTY),
        make_pair((FLAGS_testdata_dir + "/image/red.png"), RealColor::RC_RED),
        make_pair((FLAGS_testdata_dir + "/image/blue.png"), RealColor::RC_BLUE),
        make_pair((FLAGS_testdata_dir + "/image/yellow.png"), RealColor::RC_YELLOW),
        make_pair((FLAGS_testdata_dir + "/image/green.png"), RealColor::RC_GREEN),
        make_pair((FLAGS_testdata_dir + "/image/ojama.png"), RealColor::RC_OJAMA),
        make_pair((FLAGS_testdata_dir + "/image/purple.png"), RealColor::RC_PURPLE),

        make_pair((FLAGS_testdata_dir + "/image/empty-noised.png"), RealColor::RC_EMPTY),
        make_pair((FLAGS_testdata_dir + "/image/red-noised.png"), RealColor::RC_RED),
        make_pair((FLAGS_testdata_dir + "/image/blue-noised.png"), RealColor::RC_BLUE),
        make_pair((FLAGS_testdata_dir + "/image/yellow-noised.png"), RealColor::RC_YELLOW),
        make_pair((FLAGS_testdata_dir + "/image/green-noised.png"), RealColor::RC_GREEN),
        make_pair((FLAGS_testdata_dir + "/image/ojama-noised.png"), RealColor::RC_OJAMA),
        make_pair((FLAGS_testdata_dir + "/image/purple-noised.png"), RealColor::RC_PURPLE),

        make_pair((FLAGS_testdata_dir + "/image/empty-shaded.png"), RealColor::RC_EMPTY),
        make_pair((FLAGS_testdata_dir + "/image/red-shaded.png"), RealColor::RC_RED),
        make_pair((FLAGS_testdata_dir + "/image/blue-shaded.png"), RealColor::RC_BLUE),
        make_pair((FLAGS_testdata_dir + "/image/yellow-shaded.png"), RealColor::RC_YELLOW),
        make_pair((FLAGS_testdata_dir + "/image/green-shaded.png"), RealColor::RC_GREEN),
        make_pair((FLAGS_testdata_dir + "/image/ojama-shaded.png"), RealColor::RC_OJAMA),
        make_pair((FLAGS_testdata_dir + "/image/purple-shaded.png"), RealColor::RC_PURPLE),

        make_pair((FLAGS_testdata_dir + "/image/empty-blur.png"), RealColor::RC_EMPTY),
        make_pair((FLAGS_testdata_dir + "/image/red-blur.png"), RealColor::RC_RED),
        make_pair((FLAGS_testdata_dir + "/image/blue-blur.png"), RealColor::RC_BLUE),
        make_pair((FLAGS_testdata_dir + "/image/yellow-blur.png"), RealColor::RC_YELLOW),
        make_pair((FLAGS_testdata_dir + "/image/green-blur.png"), RealColor::RC_GREEN),
        make_pair((FLAGS_testdata_dir + "/image/ojama-blur.png"), RealColor::RC_OJAMA),
        make_pair((FLAGS_testdata_dir + "/image/purple-blur.png"), RealColor::RC_PURPLE),
    };

    Arow red;
    Arow blue;
    Arow yellow;
    Arow green;
    Arow purple;
    Arow ojama;
    Arow empty;

    for (int i = 0; i < 100; ++i) {
        int countFailure = 0;
        for (const auto& testcase : training_testcases) {
            const string& filename = testcase.first;
            const RealColor color = testcase.second;

            UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));

            for (int x = 0; (x + 1) * WIDTH <= surf->w; ++x) {
                for (int y = 0; (y + 1) * HEIGHT <= surf->h; ++y) {
                    int pos = 0;
                    vector<double> features(32 * 32 * 3);
                    for (int xx = 0; xx < 32; ++xx) {
                        for (int yy = 0; yy < 32; ++yy) {
                            std::uint32_t c = getpixel(surf.get(), x * WIDTH + xx, y * HEIGHT + yy);
                            std::uint8_t r, g, b;
                            SDL_GetRGB(c, surf->format, &r, &g, &b);
                            features[pos++] = r;
                            features[pos++] = g;
                            features[pos++] = b;
                        }
                    }

                    CHECK(pos == 32 * 32 * 3);

                    if (empty.update(features, color == RealColor::RC_EMPTY ? 1 : -1) != 0)
                        ++countFailure;
                    if (ojama.update(features, color == RealColor::RC_OJAMA ? 1 : -1) != 0)
                        ++countFailure;
                    if (red.update(features, color == RealColor::RC_RED ? 1 : -1) != 0)
                        ++countFailure;
                    if (blue.update(features, color == RealColor::RC_BLUE ? 1 : -1) != 0)
                        ++countFailure;
                    if (yellow.update(features, color == RealColor::RC_YELLOW ? 1 : -1) != 0)
                        ++countFailure;
                    if (green.update(features, color == RealColor::RC_GREEN ? 1 : -1) != 0)
                        ++countFailure;
                    if (purple.update(features, color == RealColor::RC_PURPLE ? 1 : -1) != 0)
                        ++countFailure;
                }
            }
        }

        cout << "done: " << i << " failure=" << countFailure << endl;
        if (countFailure == 0)
            break;
    }

    int num = 0;
    int failed = 0;

    string dirname = FLAGS_testdata_dir + "/image/actual-data";
    DIR* dir = opendir(dirname.c_str());
    PCHECK(dir);

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        string path = dirname + "/" + ent->d_name;
        if (!strings::isSuffix(path, ".png"))
            continue;

        RealColor color = toRealColor(ent->d_name[0]);

        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(path.c_str())));
        CHECK(surf.get()) << path.c_str();

        int pos = 0;
        vector<double> features(32 * 32 * 3);
        for (int x = 0; x < 32; ++x) {
            for (int y = 0; y < 32; ++y) {
                std::uint32_t c = getpixel(surf.get(), x, y);
                std::uint8_t r, g, b;
                SDL_GetRGB(c, surf->format, &r, &g, &b);
                features[pos++] = r;
                features[pos++] = g;
                features[pos++] = b;
            }
        }

        CHECK(pos == 32 * 32 * 3);

        static const RealColor colors[] = {
            RealColor::RC_EMPTY, RealColor::RC_OJAMA, RealColor::RC_RED,
            RealColor::RC_BLUE, RealColor::RC_YELLOW, RealColor::RC_GREEN, RealColor::RC_PURPLE
        };

        double margins[] = {
            empty.margin(features), ojama.margin(features), red.margin(features),
            blue.margin(features), yellow.margin(features), green.margin(features), purple.margin(features)
        };

        ++num;

        int idx = max_element(margins, margins + 7) - margins;
        if (color != colors[idx]) {
            cout << "FAILED: " << color << " -> " << colors[idx] << " : " << path << endl;
            for (int i = 0; i < 7; ++i) {
                cout << colors[i] << " :-> " << margins[i] << endl;
            }
            ++failed;
        }
    }
    closedir(dir);

    cout << "num = " << num << endl;
    cout << "failed =" << failed << endl;
    cout << double(num - failed) / num << endl;
}
