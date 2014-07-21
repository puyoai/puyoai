#include "book_reader.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;

static string trim(const string& str)
{
    string::size_type left = str.find_first_not_of(" ");
    if (left == string::npos)
        return str;

    string::size_type right = str.find_last_not_of(" ");
    return str.substr(left, right - left + 1);
}

static vector<string> split(const string& str)
{
    vector<string> result;

    istringstream ss(str);
    string s;
    while (ss >> s) {
        result.push_back(s);
    }

    return result;
}

// static
vector<BookField> BookReader::parse(const string& filename)
{
    ifstream ifs(filename);

    vector<BookField> result;

    multimap<string, BookField> partialFields;
    string currentName;
    vector<string> currentField;
    double score = 1.0;

    string str;
    while (getline(ifs, str)) {
        if (str.empty())
            continue;
        if (str[0] == '#')
            continue;
        if (str.find("NAME:") == 0) {
            if (!currentName.empty())
                partialFields.emplace(currentName, BookField(currentName, currentField, score));

            currentName = trim(str.substr(5));
            currentField.clear();
            score = 1.0;
            continue;
        }
        if (str.find("SCORE:") == 0) {
            score = stof(str.substr(6));
            continue;
        }
        if (str.size() == 6) {
            currentField.push_back(str);
            continue;
        }

        if (str.find("COMBINE:") == 0) {
            if (!currentName.empty()) {
                partialFields.emplace(currentName, BookField(currentName, currentField, score));
                currentName = trim(str.substr(5));
                currentField.clear();
                score = 1.0;
            }

            // TODO(mayah): awful bad code.
            vector<string> names = split(trim(str.substr(8)));
            CHECK(1 <= names.size() && names.size() <= 3);
            if (names.size() == 1) {
                auto range = partialFields.equal_range(names[0]);
                CHECK(range.first != range.second);
                for (auto it = range.first; it != range.second; ++it) {
                    result.push_back(it->second);
                    result.push_back(it->second.mirror());
                }
            } else if (names.size() == 2) {
                auto range1 = partialFields.equal_range(names[0]);
                auto range2 = partialFields.equal_range(names[1]);
                CHECK(range1.first != range1.second);
                CHECK(range2.first != range2.second);
                for (auto it = range1.first; it != range1.second; ++it) {
                    for (auto jt = range2.first; jt != range2.second; ++jt) {
                        BookField bf(it->second);
                        bf.merge(jt->second);
                        result.push_back(bf);
                        result.push_back(bf.mirror());
                    }
                }
            } else if (names.size() == 3) {
                auto range1 = partialFields.equal_range(names[0]);
                auto range2 = partialFields.equal_range(names[1]);
                auto range3 = partialFields.equal_range(names[2]);
                CHECK(range1.first != range1.second);
                CHECK(range2.first != range2.second);
                CHECK(range3.first != range3.second);
                for (auto it = range1.first; it != range1.second; ++it) {
                    for (auto jt = range2.first; jt != range2.second; ++jt) {
                        for (auto kt = range3.first; kt != range3.second; ++kt) {
                            BookField bf(it->second);
                            bf.merge(jt->second);
                            bf.merge(kt->second);
                            result.push_back(bf);
                            result.push_back(bf.mirror());
                        }
                    }
                }
            }
            continue;
        }

        CHECK(false) << "Unexpected line: " << str;
    }

    return result;
}
