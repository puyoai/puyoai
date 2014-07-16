#include "book_reader.h"

#include <iostream>
#include <fstream>
#include <map>

using namespace std;

static string trim(const string& str)
{
    string::size_type left = str.find_first_not_of(" ");
    if (left == string::npos)
        return str;

    string::size_type right = str.find_last_not_of(" ");
    return str.substr(left, right - left + 1);
}

static void addBookField(const string& name, const string& with, const vector<string>& f, bool partial, multimap<string, BookField>* mm)
{
    if (with.empty()) {
        mm->emplace(name, BookField(name, f, partial));
        return;
    }

    CHECK(mm->count(with)) << "Unknown WITH-name " << with;

    vector<BookField> bfs;
    auto range = mm->equal_range(with);
    for (auto it = range.first; it != range.second; ++it) {
        // Do not modify mm in this loop.
        BookField bf(name, f, partial);
        bf.merge(it->second);
        bfs.push_back(bf);
    }

    for (const auto& bf : bfs)
        mm->emplace(name, bf);
}

// static
vector<BookField> BookReader::parse(const string& filename)
{
    ifstream ifs(filename);

    multimap<string, BookField> m;

    string currentTypeName;
    string currentWithName;
    vector<string> currentField;
    bool partial = false;

    string str;
    while (getline(ifs, str)) {
        if (str.empty())
            continue;
        if (str[0] == '#')
            continue;
        if (str.find("TYPE:") == 0) {
            if (!currentTypeName.empty())
                addBookField(currentTypeName, currentWithName, currentField, partial, &m);

            currentTypeName = trim(str.substr(5));
            currentWithName = "";
            currentField.clear();
            partial = false;
            continue;
        }
        if (str.find("WITH:") == 0) {
            currentWithName = trim(str.substr(5));
            continue;
        }
        if (str.find("PARTIAL:") == 0) {
            partial = true;
            continue;
        }
        if (str.size() == 6) {
            currentField.push_back(str);
            continue;
        }

        CHECK(false) << "Unexpected line: " << str;
    }

    LOG(INFO) << "ghoe" << endl;

    if (!currentTypeName.empty())
        addBookField(currentTypeName, currentWithName, currentField, partial, &m);

    vector<BookField> result;
    for (const auto& entry : m) {
        if (!entry.second.isPartial()) {
            result.push_back(entry.second);
            result.push_back(entry.second.mirror());
        }
    }

    return result;
}
