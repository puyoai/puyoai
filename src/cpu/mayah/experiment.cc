#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <toml/toml.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/client/ai/solver/solver.h"

#include "evaluation_parameter.h"

DEFINE_string(feature, SRC_DIR "/cpu/mayah/feature.toml", "the path to feature parameter");

using namespace std;

void iter(int pos, char* s, int used, vector<string>* strs)
{
    if (pos == 10) {
        strs->push_back(s);
        return;
    }

    for (int i = 0; i <= used; ++i) {
        if (i >= 4)
            continue;
        if (pos <= 6 && i >= 3)
            continue;
        s[pos] = 'A' + i;
        if ((pos & 1) && s[pos - 1] > s[pos]) {
            continue;
        }
        iter(pos + 1, s, std::max(used, i + 1), strs);
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    EvaluationParameter param(FLAGS_feature);
    param.save("hoge.toml");

    vector<string> all;
    char str[11];
    str[10] = '\0';
    iter(0, str, 0, &all);

    toml::Value root;
    for (const auto& str : all) {
        toml::Value* parent = &root;
        for (int i = 0; i < 4; ++i) {
            string s = str.substr(i * 2, 4);

            if (!parent->find(s)) {
                parent->set(s, toml::Value(toml::Table()));
            }

            parent = parent->find(s);
        }
    }

    cout << root;

    return 0;
}
