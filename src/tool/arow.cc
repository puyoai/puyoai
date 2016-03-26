#include <algorithm>
#include <fstream>
#include <vector>

#include <dirent.h>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <SDL_image.h>

#include "base/base.h"
#include "base/strings.h"
#include "capture/color.h"
#include "capture/recognition/arow.h"
#include "core/real_color.h"
#include "gui/unique_sdl_surface.h"
#include "gui/util.h"

DECLARE_string(testdata_dir);

using namespace std;

const char* COLOR_NAMES[] = {
    "RED", "BLUE", "YELLOW", "GREEN", "PURPLE", "EMPTY", "OJAMA", "ZENKESHI"
};

const int IMAGE_WIDTH = 16;
const int IMAGE_HEIGHT = 16;

bool readFeatures(vector<vector<double>> features[NUM_RECOGNITION])
{
    const pair<string, RecognitionColor> training_testcases[] = {
        make_pair((FLAGS_testdata_dir + "/images/puyo/red.png"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue.png"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow.png"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green.png"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple.png"), RecognitionColor::PURPLE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty.png"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama.png"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/zenkeshi.png"), RecognitionColor::ZENKESHI),

        make_pair((FLAGS_testdata_dir + "/images/puyo/red-blur.png"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue-blur.png"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow-blur.png"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green-blur.png"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple-blur.png"), RecognitionColor::PURPLE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty-blur.png"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama-blur.png"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/zenkeshi-blur.png"), RecognitionColor::ZENKESHI),

        make_pair((FLAGS_testdata_dir + "/images/puyo/red-actual.png"), RecognitionColor::RED),
        make_pair((FLAGS_testdata_dir + "/images/puyo/blue-actual.png"), RecognitionColor::BLUE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/yellow-actual.png"), RecognitionColor::YELLOW),
        make_pair((FLAGS_testdata_dir + "/images/puyo/green-actual.png"), RecognitionColor::GREEN),
        make_pair((FLAGS_testdata_dir + "/images/puyo/purple-actual.png"), RecognitionColor::PURPLE),
        make_pair((FLAGS_testdata_dir + "/images/puyo/empty-actual.png"), RecognitionColor::EMPTY),
        make_pair((FLAGS_testdata_dir + "/images/puyo/ojama-actual.png"), RecognitionColor::OJAMA),
        make_pair((FLAGS_testdata_dir + "/images/puyo/zenkeshi-actual.png"), RecognitionColor::ZENKESHI),
    };

    // Read training testcases.
    for (const auto& testcase: training_testcases) {
        const string& filename = testcase.first;
        const RecognitionColor color = testcase.second;

        cout << "Opening... " << filename << endl;
        UniqueSDLSurface surf(makeUniqueSDLSurface(IMG_Load(filename.c_str())));
        CHECK(surf.get());

        for (int x = 0; (x + 1) * IMAGE_WIDTH <= surf->w; ++x) {
            for (int y = 0; (y + 1) * IMAGE_HEIGHT <= surf->h; ++y) {
                int pos = 0;
                vector<double> fs(IMAGE_WIDTH * IMAGE_HEIGHT * 3);
                for (int yy = 0; yy < IMAGE_HEIGHT; ++yy) {
                    for (int xx = 0; xx < IMAGE_WIDTH; ++xx) {
                        std::uint32_t c = getpixel(surf.get(), x * IMAGE_WIDTH + xx, y * IMAGE_HEIGHT + yy);
                        std::uint8_t r, g, b;
                        SDL_GetRGB(c, surf->format, &r, &g, &b);
                        fs[pos++] = r;
                        fs[pos++] = g;
                        fs[pos++] = b;
                    }
                }

                CHECK(pos == IMAGE_WIDTH * IMAGE_HEIGHT * 3);
                features[static_cast<int>(color)].push_back(std::move(fs));
            }
        }
    }

    for (int i = 0; i < NUM_RECOGNITION; ++i) {
        const auto& f = features[i];
        cout << "SIZE = " << f.size() << endl;
    }

    return true;
}

int main()
{
    vector<vector<double>> features[NUM_RECOGNITION];
    CHECK(readFeatures(features));

#if 0
    const int LEARNING_X_BEGIN = 0;
    const int LEARNING_X_END = 16;
    const int LEARNING_WIDTH = LEARNING_X_END - LEARNING_X_BEGIN;
    const int LEARNING_HEIGHT = IMAGE_HEIGHT;

    const char COLOR_NAME_PREFIX[] = "";
    const int RECOGNITION_SIZE = 8;
#endif

#if 1
    const int LEARNING_X_BEGIN = 0;
    const int LEARNING_X_END = 8;
    const int LEARNING_WIDTH = LEARNING_X_END - LEARNING_X_BEGIN;
    const int LEARNING_HEIGHT = IMAGE_HEIGHT;

    const char COLOR_NAME_PREFIX[] = "LEFT_";
    const int RECOGNITION_SIZE = 6;
#endif

#if 0
    const int LEARNING_X_BEGIN = 8;
    const int LEARNING_X_END = 16;
    const int LEARNING_WIDTH = LEARNING_X_END - LEARNING_X_BEGIN;
    const int LEARNING_HEIGHT = IMAGE_HEIGHT;

    const char COLOR_NAME_PREFIX[] = "RIGHT_";
    const int RECOGNITION_SIZE = 6;
#endif

    unique_ptr<Arow> arows[RECOGNITION_SIZE];
    for (int i = 0; i < RECOGNITION_SIZE; ++i) {
        arows[i].reset(new Arow(LEARNING_WIDTH * LEARNING_HEIGHT * 3));
    }

    // training
    for (int times = 0; times < 300; ++times) {
        if (times == 200) {
            for (int i = 0; i < RECOGNITION_SIZE; ++i) {
                arows[i]->setRate(0.02);
            }
        }

        if (times == 250) {
            for (int i = 0; i < RECOGNITION_SIZE; ++i) {
                arows[i]->setRate(0.005);
            }
        }

        for (int i = 0; i < RECOGNITION_SIZE; ++i) {
            for (int j = 0; j < static_cast<int>(features[i].size()); ++j) {
                if ((j & 0xF) == 0)
                    continue;

                for (int k = 0; k < RECOGNITION_SIZE; ++k) {
                    arows[k]->update(features[i][j], i == k ? 1 : -1);
                }
            }
        }

        cout << "training " << times << ": done" << endl;
    }

    // test by all
    int num = 0;
    int fail = 0;
    for (int i = 0; i < RECOGNITION_SIZE; ++i) {
        for (int j = 0; j < static_cast<int>(features[i].size()); ++j) {
            ++num;
            double vs[RECOGNITION_SIZE] {};
            for (int k = 0; k < RECOGNITION_SIZE; ++k) {
                vs[k] = arows[k]->margin(features[i][j]);
            }

            int result = std::max_element(vs, vs + RECOGNITION_SIZE) - vs;
            if (i != result) {
                cout << "fail: " << i << " " << j << " -> " << result << endl;
                for (int k = 0; k < RECOGNITION_SIZE; ++k)
                    cout << vs[k] << ' ';
                cout << endl;
                ++fail;
            }
        }
    }

    cout << "num = " << num << endl;
    cout << "fail = " << fail << endl;

    // Save the data as C-array.
    std::stringstream body;
    {
        body << "#include <cstddef>" << endl;
        body << "#include <cstdint>" << endl;

        for (size_t i = 0; i < RECOGNITION_SIZE; ++i) {
            body << "const std::size_t " << COLOR_NAME_PREFIX << COLOR_NAMES[i] << "_MEAN_SIZE = "
                 << arows[i]->mean().size() << ";" << endl;
            body << "const std::size_t " << COLOR_NAME_PREFIX << COLOR_NAMES[i] << "_COV_SIZE = "
                 << arows[i]->cov().size() << ";" << endl;
        }

        for (size_t i = 0; i < RECOGNITION_SIZE; ++i) {
            body << endl;
            body << "const double " << COLOR_NAME_PREFIX << COLOR_NAMES[i] << "_MEAN[] = {" << endl;
            for (size_t j = 0; j < arows[i]->mean().size(); ++j) {
                if (j % 4 == 0) {
                    body << "    ";
                } else {
                    body << " ";
                }
                body << scientific << showpos << setprecision(16) << arows[i]->mean()[j] << ",";
                if (j % 4 == 3) {
                    body << std::endl;
                }
            }
            body << "};" << endl;
        }

        for (size_t i = 0; i < RECOGNITION_SIZE; ++i) {
            body << endl;
            body << "const double " << COLOR_NAME_PREFIX << COLOR_NAMES[i] << "_COV[] = {" << endl;
            for (size_t j = 0; j < arows[i]->cov().size(); ++j) {
                if (j % 4 == 0) {
                    body << "    ";
                } else {
                    body << " ";
                }
                body << scientific << showpos << setprecision(16) << arows[i]->cov()[j] << ",";
                if (j % 4 == 3) {
                    body << std::endl;
                }
            }
            body << "};" << endl;
        }
    }

    ofstream ofs("classifier_features.cc");
    ofs << body.str();
    ofs.close();

    return 0;
}
