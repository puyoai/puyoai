#include "book_reader.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static string trim(const string& str)
{
    string::size_type left = str.find_first_not_of(" ");
    if (left == string::npos)
        return str;

    string::size_type right = str.find_last_not_of(" ");
    return str.substr(left, right - left + 1);
}

void BookReader::parse(const string& filename)
{
    ifstream ifs(filename);

    string currentTypeName;
    string currentWithName;
    vector<string> currentField;

    string str;
    while (getline(ifs, str)) {
        if (str.empty())
            continue;
        if (str[0] == '#')
            continue;
        if (str.find("TYPE:") == 0) {
            if (!currentTypeName.empty()) {
                cout << "name = " << currentTypeName << endl;
                cout << "with = " << currentWithName << endl;
                for (const auto& s : currentField)
                    cout << s << endl;
            }

            currentTypeName = trim(str.substr(5));
            currentWithName = "";
            currentField.clear();
            continue;
        }
        if (str.find("WITH:") == 0) {
            currentWithName = trim(str.substr(5));
            continue;
        }
        if (str.size() == 6) {
            currentField.push_back(str);
            continue;
        }

        CHECK(false) << "Unexpected line: " << str;
    }
}
