#include "book_reader.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

#include <toml/toml.h>

#include "base/strings.h"
using namespace std;

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
        if (!field.merge(it->second))
            continue;
        merge(result, field, partialFields, names, pos + 1);
    }
}

// static
vector<BookField> BookReader::parse(const string& filename)
{
    vector<BookField> result;
    multimap<string, BookField> partialFields;

    ifstream ifs(filename);
    toml::Value value;
    try {
        toml::Parser parser(ifs);
        value = parser.parse();
        if (!value.valid()) {
            LOG(ERROR) << parser.errorReason();
            return vector<BookField>();
        }
    } catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return vector<BookField>();
    }

    const toml::Array& books = value.find("book")->as<toml::Array>();
    for (const auto& book : books) {
        string name = book.get<string>("name");
        double score = book.get<double>("score");
        vector<string> field;
        for (const auto& s : book.get<toml::Array>("field"))
            field.push_back(s.as<string>());

        partialFields.emplace(name, BookField(name, field, score));
    }

    const toml::Array& combines = value.find("combine")->as<toml::Array>();
    for (const auto& combine : combines) {
        string combinedName = combine.get<string>("name");
        vector<string> names;
        for (const auto& s : combine.get<toml::Array>("combine")) {
            names.push_back(s.as<string>());
        }

        auto range = partialFields.equal_range(names[0]);
        for (auto it = range.first; it != range.second; ++it) {
            merge(&result, it->second, partialFields, names, 1);
        }
    }

    return result;
}
