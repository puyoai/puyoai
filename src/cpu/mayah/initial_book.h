#ifndef CPU_MAYAH_INITIAL_BOOK_H_
#define CPU_MAYAH_INITIAL_BOOK_H_

#include <string>
#include <map>

#include <toml/toml.h>

#include "core/decision.h"
#include "core/field_constant.h"
#include "core/puyo_color.h"

class CoreField;
class KumipuyoSeq;

class InitialBookField : FieldConstant {
public:
    InitialBookField(const std::vector<std::string>& field,
                     std::map<std::string, Decision>&& decisions);

    Decision nextDecision(const CoreField&, const KumipuyoSeq&) const;

private:
    int8_t heights_[MAP_WIDTH];
    char field_[MAP_WIDTH][MAP_HEIGHT];
    std::map<std::string, Decision> decisions_;
};

class InitialBook {
public:
    InitialBook();
    explicit InitialBook(const std::string& filename);

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    // Finds next decision. If next decision is not found, invalid Decision will be returned.
    Decision nextDecision(const CoreField&, const KumipuyoSeq&);

private:
    void makeFieldFromValue(const CoreField&, const std::string&, const toml::Value&);

    std::vector<InitialBookField> fields_;
};

#endif
