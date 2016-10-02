#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <json/json.h>

#include "core/core_field.h"
#include "core/pattern/pattern_book.h"
#include "core/rensa_tracker/rensa_chain_tracker.h"

using namespace std;

int index(int x, int y)
{
    return (12 - y) * 6 + (x - 1);
}

void add(PatternBook* patternBook, const CoreField& original, std::unordered_set<std::string>* patterns)
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
            stringstream ss;
            ss << "[[pattern]]" << endl;
            ss << "field = [" << endl;
            for (size_t i = 0; i < s.size(); ++i) {
                if (i % 6 == 0)
                    ss << "    \"";
                ss << s[i];
                if (i % 6 == 5)
                    ss << "\"," << endl;
            }
            ss << "]" << endl;

            string s = ss.str();
            patterns->insert(s);
            // cout << s << endl;
            patternBook->loadFromString(s, true);
        }

        current.vanishDropFast();
    }
}

bool parseAndAdd(const char* filename, PatternBook* patternBook,
                 std::unordered_set<std::string>* patterns,
                 std::unordered_set<CoreField>* visited)
{
    ifstream ifs(filename);
    if (!ifs) {
        PLOG(ERROR) << "failed to open " << filename;
        return false;
    }

    Json::Value root;
    ifs >> root;

    for (unsigned int i = 0; i < root.size(); ++i) {
        PlainField pf(root[i]["p1"].asString());
        CoreField cf(CoreField::fromPlainFieldWithDrop(pf));
        if (!visited->insert(cf).second || !cf.rensaWillOccur())
            continue;

        add(patternBook, cf, patterns);
    }
    return true;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (argc < 2) {
        cerr << argv[0] << " <filename> ..." << endl;
        return EXIT_FAILURE;
    }

    PatternBook patternBook;
    unordered_set<CoreField> visited;
    std::unordered_set<std::string> patterns;
    for (char** filename = argv + 1; *filename; ++filename) {
        // cout << *filename << endl;
        if (!parseAndAdd(*filename, &patternBook, &patterns, &visited)) {
            cerr << "failed to add: " << *filename;
        }
    }

    for (const auto& entry : patterns) {
        cout << entry << endl;
    }

    return 0;
}
