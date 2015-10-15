#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <json/json.h>

#include "core/core_field.h"
#include "core/rensa_tracker/rensa_chain_tracker.h"

using namespace std;

int index(int x, int y)
{
    return (12 - y) * 6 + (x - 1);
}

void add(const CoreField& original)
{
    CoreField current(original);
    while (true) {
        CoreField cf(current);
        RensaChainTracker tracker;
        if (cf.simulateFast(&tracker) < 3)
            break;

        FieldBits bits[3];

        string s(72, '.'); // right ?
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                int r = tracker.result().erasedAt(x, y);
                if (r < 1 || 3 < r)
                    continue;
                s[index(x, y)] = static_cast<char>('A' + (r - 1));
                bits[r - 1].set(x, y);
            }
        }

        bool ok = true;
        for (int i = 0; i < 3; ++i) {
            PuyoColor pc = PuyoColor::EMPTY;
            if (!(current.bitField().bits(PuyoColor::OJAMA) & bits[i]).isEmpty()) {
                ok = false;
                break;
            }

            for (PuyoColor c : NORMAL_PUYO_COLORS) {
                if (!(current.bitField().bits(c) & bits[i]).isEmpty()) {
                    if (pc == PuyoColor::EMPTY) {
                        pc = c;
                    } else {
                        ok = false;
                        break;
                    }
                }
            }
        }

        if (ok) {
            cout << "######" << endl;
            for (size_t i = 0; i < s.size(); ++i) {
                cout << s[i];
                if (i % 6 == 5)
                    cout << endl;
            }
            cout << "######" << endl;
        }

        current.vanishDropFast();
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        cerr << argv[0] << " <filename>" << endl;
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    ifstream ifs(filename);
    if (!ifs) {
        PLOG(ERROR) << "failed to open " << filename;
        return EXIT_FAILURE;
    }

    Json::Value root;
    ifs >> root;

    cout << root.size() << endl;

    unordered_set<CoreField> visited;
    for (unsigned int i = 0; i < root.size(); ++i) {
        PlainField pf(root[i]["p1"].asString());
        CoreField cf(CoreField::fromPlainFieldWithDrop(pf));
        if (!visited.insert(cf).second || !cf.rensaWillOccur())
            continue;

        add(cf);
    }
    return EXIT_SUCCESS;
}
