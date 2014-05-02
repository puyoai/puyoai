#ifndef __FIELD_H__
#define __FIELD_H__

#include <string>
#include <vector>

#include "basic_field.h"
#include "puyo.h"
#include "puyo_set.h"
#include "util.h"

class Decision;
class FieldBitField;
class FieldColumnBitField;
class NonTrackingStrategy;
class RensaTrackResult;
class TrackingStrategy;
struct BasicRensaInfo;
struct TrackedRensaInfo;
struct Position;

class Field : public BasicField {
public:
    Field() : BasicField() {}
    Field(const std::string& url) : BasicField(url) {}
    Field(const Field& field) : BasicField(field) {}

    // Check if the field is in Zenkeshi
    bool isZenkeshi() const;
    int countColorPuyos() const;
    int countPuyos() const;
    int connectedPuyoNums(int x, int y) const;
    int connectedPuyoNums(int x, int y, FieldBitField& checked) const;
    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y) const;
    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y, FieldBitField& checked) const;

    void dropKumiPuyoSafely(const Decision&, const KumiPuyo&);

    bool findBestBreathingSpace(int& breathingX, int& breathingY, int x, int y) const;

    void showDebugOutput() const;
    
    // Compatibility interface for Ctrl
    PuyoColor Get(int x, int y) const { return color(x, y); }

    friend bool operator==(const Field&, const Field&);
};

class ArbitrarilyModifiableField : public Field {
public:
    ArbitrarilyModifiableField() {}
    ArbitrarilyModifiableField(const Field& field) :
        Field(field) {}

    // Do not use not in PlayerInfo.    
    using Field::setPuyo;
    using Field::recalcHeightOn;
};

#endif  // __FIELD_H__
