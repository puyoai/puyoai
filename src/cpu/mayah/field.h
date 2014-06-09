#ifndef CPU_MAYAH_FIELD_H__
#define CPU_MAYAH_FIELD_H__

#include <string>
#include <vector>

#include "base/base.h"
#include "core/field/core_field.h"

class Decision;
class FieldBitField;

class Field : public CoreField {
public:
    Field() : CoreField() {}
    Field(const std::string& url) : CoreField(url) {}
    Field(const Field& field) : CoreField(field) {}
    Field(const CoreField& field) : CoreField(field) {}
    Field(const PlainField& field) : CoreField(field) {}

    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y) const;
    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y, FieldBitField& checked) const;

    void dropKumipuyoSafely(const Decision&, const Kumipuyo&);

    bool findBestBreathingSpace(int& breathingX, int& breathingY, int x, int y) const;

    void showDebugOutput() const;
};

inline const Field& toField(const CoreField& f) {
    static_assert(sizeof(Field) == sizeof(CoreField), "size of CoreField and Field should be same.");
    return reinterpret_cast<const Field&>(f);
}

class ArbitrarilyModifiableField : public Field {
public:
    ArbitrarilyModifiableField() {}
    ArbitrarilyModifiableField(const Field& field) : Field(field) {}
    ArbitrarilyModifiableField(const CoreField& field) : Field(field) {}

    // Do not use not in PlayerInfo.
    // TODO(mayah): Use unsafeSet directly.
    void setPuyo(int x, int y, PuyoColor c) {
        unsafeSet(x, y, c);
    }
    using Field::recalcHeightOn;
};

#endif  // CPU_MAYAH_FIELD_H_
