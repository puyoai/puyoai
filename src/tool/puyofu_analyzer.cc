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

void add(const CoreField& original)
{
    CoreField current(original);
    while (true) {
        CoreField cf(current);
        RensaChainTracker tracker;
        if (cf.simulateFast(&tracker) < 3)
            break;

        // TODO(mayah): write this.
        // string s(72, ' '); // right ?
        // for (int x = 1; x <= 6; ++x) {
        //     for (int y = 1; y <= 12; ++y) {
        //         switch (tracker.result().erasedAt(x, y)) {
        //             case 1:
        //                 s[(]
        //                 break;
        //             case 2:
        //                 break;
        //             case 3:
        //                 break;
        //             default:
        //                 break;
        //         }
        //     }
        // }
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
