#ifndef CAPTURE_CAPTURE_SOURCE_H_
#define CAPTURE_CAPTURE_SOURCE_H_

inline void convertUVY2RGBA(int u, int v, int y, int* r, int* g, int* b)
{
    u -= 128;
    v -= 128;

    double r1 = y + 1.40200 * v;
    double g1 = y - 0.34414 * u - 0.71414 * v;
    double b1 = y + 1.77200 * u;

    r1 = (r1 - 16) * 255 / 219;
    g1 = (g1 - 16) * 255 / 219;
    b1 = (b1 - 16) * 255 / 219;

    *r = std::max(0, std::min(255, static_cast<int>(r1)));
    *g = std::max(0, std::min(255, static_cast<int>(g1)));
    *b = std::max(0, std::min(255, static_cast<int>(b1)));
}

#endif
