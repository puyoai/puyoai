#ifndef CORE_ALGORITHM_PUYO_SET_H_
#define CORE_ALGORITHM_PUYO_SET_H_

#include <stdio.h>
#include <string>

#include "core/puyo_color.h"

class ColumnPuyoList;

class PuyoSet {
public:
    PuyoSet() :
        red_(0),
        blue_(0),
        yellow_(0),
        green_(0)
    {
    }

    PuyoSet(int red, int blue, int yellow, int green)
    {
        red_ = red;
        blue_ = blue;
        yellow_ = yellow;
        green_ = green;
    }

    std::string toString() const
    {
        char buf[80];
        sprintf(buf, "(%d, %d, %d, %d)", red(), blue(), yellow(), green());

        return buf;
    }

    int count() const { return red_ + blue_ + yellow_ + green_; }

    int red() const { return red_; }
    int blue() const { return blue_; }
    int yellow() const { return yellow_; }
    int green() const { return green_; }

    void add(const ColumnPuyoList&);

    void add(PuyoSet set)
    {
        red_ += set.red_;
        blue_ += set.blue_;
        yellow_ += set.yellow_;
        green_ += set.green_;
    }

    void sub(PuyoSet set)
    {
        red_ = red_ < set.red_ ? 0 : red_ - set.red_;
        blue_ = blue_ < set.blue_ ? 0 : blue_ - set.blue_;
        yellow_ = yellow_ < set.yellow_ ? 0 : yellow_ - set.yellow_;
        green_ = green_ < set.green_ ? 0 : green_ - set.green_;
    }

    void add(PuyoColor c, int n = 1)
    {
        switch (c) {
        case PuyoColor::RED:
            red_ += n;
            break;
        case PuyoColor::BLUE:
            blue_ += n;
            break;
        case PuyoColor::YELLOW:
            yellow_ += n;
            break;
        case PuyoColor::GREEN:
            green_ += n;
            break;
        default:
            DCHECK(false);
        }
    }

    friend bool operator<(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        return lhs.toInt() < rhs.toInt();
    }

    friend bool operator==(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        return lhs.toInt() == rhs.toInt();
    }

    friend bool operator!=(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        return lhs.toInt() != rhs.toInt();
    }

private:
    int toInt() const
    {
        return red_ | (blue_ << 4) | (yellow_ << 8) | (green_ << 12);
    }

    int red_;
    int blue_;
    int yellow_;
    int green_;
};

#endif
