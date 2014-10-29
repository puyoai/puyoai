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

static void merge(vector<BookField>* result,
                  const BookField& current,
                  const multimap<string, BookField>& partialFields,
                  const vector<string>& names,
                  size_t pos)
{
    if (pos == names.size()) {
        result->push_back(current);
        result->push_back(current.mirror());
        return;
    }

    auto range = partialFields.equal_range(names[pos]);
    for (auto it = range.first; it != range.second; ++it) {
        BookField field(current);
        field.merge(it->second);
        merge(result, field, partialFields, names, pos + 1);
    }
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
            CHECK(names.size() > 0);
            {
                auto range = partialFields.equal_range(names[0]);
                for (auto it = range.first; it != range.second; ++it) {
                    merge(&result, it->second, partialFields, names, 1);
                }
            }
            continue;
        }

        CHECK(false) << "Unexpected line: " << str;
    }

    return result;
}
