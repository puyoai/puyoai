#ifndef CAPTURE_COLOR_H_
#define CAPTURE_COLOR_H_

struct HSV;
struct RGB;

// We use cone model of HSV.
struct HSV {
    HSV() : h(0), s(0), v(0) {}
    HSV(float h, float s, float v) : h(h), s(s), v(v) {}

    RGB toRGB() const;

    float h, s, v;
};

struct RGB {
    RGB() : r(0), g(0), b(0) {}
    RGB(float r, float g, float b) : r(r), g(g), b(b) {}

    HSV toHSV() const;

    float r, g, b;
};

#endif
