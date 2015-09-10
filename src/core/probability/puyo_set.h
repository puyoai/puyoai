#ifndef CORE_PROBABILITY_PUYO_SET_H_
#define CORE_PROBABILITY_PUYO_SET_H_

#include <stdio.h>
#include <string>

#include "core/puyo_color.h"

class ColumnPuyoList;

class PuyoSet {
public:
    PuyoSet() : PuyoSet(0, 0, 0, 0)
    {
    }

    PuyoSet(int red, int blue, int yellow, int green) :
        red_(red),
        blue_(blue),
        yellow_(yellow),
        green_(green)
    {
    }

    explicit PuyoSet(const ColumnPuyoList& columnPuyoList) : PuyoSet()
    {
        add(columnPuyoList);
    }

    std::string toString() const
    {
        char buf[80];
        sprintf(buf, "(%d, %d, %d, %d)", red(), blue(), yellow(), green());

        return buf;
    }

    int count() const { return red_ + blue_ + yellow_ + green_; }
    bool isEmpty() const { return count() == 0; }

    int red() const { return red_; }
    int blue() const { return blue_; }
    int yellow() const { return yellow_; }
    int green() const { return green_; }

    void add(const ColumnPuyoList&);

    void add(const PuyoSet& set)
    {
        red_ += set.red_;
        blue_ += set.blue_;
        yellow_ += set.yellow_;
        green_ += set.green_;
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
            break;
        }
    }

    void sub(const PuyoSet& set)
    {
        red_ = red_ < set.red_ ? 0 : red_ - set.red_;
        blue_ = blue_ < set.blue_ ? 0 : blue_ - set.blue_;
        yellow_ = yellow_ < set.yellow_ ? 0 : yellow_ - set.yellow_;
        green_ = green_ < set.green_ ? 0 : green_ - set.green_;
    }

    void sub(PuyoColor c)
    {
        switch (c) {
        case PuyoColor::RED:
            if (red_ > 0)
                red_ -= 1;
            break;
        case PuyoColor::BLUE:
            if (blue_ > 0)
                blue_ -= 1;
            break;
        case PuyoColor::YELLOW:
            if (yellow_ > 0)
                yellow_ -= 1;
            break;
        case PuyoColor::GREEN:
            if (green_ > 0)
                green_ -= 1;
            break;
        default:
            break;
        }
    }

    friend bool operator<(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        if (lhs.red_ != rhs.red_)
            return lhs.red_ < rhs.red_;
        if (lhs.blue_ != rhs.blue_)
            return lhs.blue_ < rhs.blue_;
        if (lhs.yellow_ != rhs.yellow_)
            return lhs.yellow_ < rhs.yellow_;
        return lhs.green_ < rhs.green_;
    }

    friend bool operator==(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        return lhs.red_ == rhs.red_ &&
            lhs.blue_ == rhs.blue_ &&
            lhs.yellow_ == rhs.yellow_ &&
            lhs.green_ == rhs.green_;
    }

    friend bool operator!=(const PuyoSet& lhs, const PuyoSet& rhs)
    {
        return !(lhs == rhs);
    }

private:
    int red_ = 0;
    int blue_ = 0;
    int yellow_ = 0;
    int green_ = 0;
};

#endif // CORE_PROBABILITY_PUYO_SET_H_
