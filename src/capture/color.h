#ifndef CAPTURE_COLOR_H_
#define CAPTURE_COLOR_H_

#include <algorithm>
#include <cmath>

struct HSV;
struct RGB;

// We use cone model of HSV.
struct HSV {
    HSV() : h(0), s(0), v(0) {}
    HSV(float h, float s, float v) : h(h), s(s), v(v) {}

    RGB toRGB() const;
    static double diff2(const HSV&, const HSV&);

    float h, s, v;
};

struct RGB {
    RGB() : r(0), g(0), b(0) {}
    RGB(float r, float g, float b) : r(r), g(g), b(b) {}

    void add(const RGB& c)
    {
        r += c.r;
        g += c.g;
        b += c.b;
    }

    void div(float f)
    {
        if (f) {
            r /= f;
            g /= f;
            b /= f;
        } else {
            r = g = b = 0;
        }
    }

    HSV toHSV() const;
    float r, g, b;
};

inline float floatDiff2(float f1, float f2)
{
    return (f1 - f2) * (f1 - f2);
}

inline float colorDiff2(const RGB& c1, const RGB& c2)
{
    return floatDiff2(c1.r, c2.r) + floatDiff2(c1.g, c2.g) + floatDiff2(c1.b, c2.b);
}

#endif
