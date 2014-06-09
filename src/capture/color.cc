#include "capture/color.h"

#include <algorithm>
#include <glog/logging.h>

using namespace std;

HSV RGB::toHSV() const
{
    float mx = max(max(r, g), b);
    float mn = min(min(r, g), b);

    HSV hsv;

    if (mx == mn)
        hsv.h = 180;
    else if (mx == r)
        hsv.h = 60 * (g - b) / (mx - mn);
    else if (mx == g)
        hsv.h = 60 * (b - r) / (mx - mn) + 120;
    else if (mx == b)
        hsv.h = 60 * (r - g) / (mx - mn) + 240;
    else
        DCHECK(false);

    while (hsv.h < 0)
        hsv.h += 360;
    while (hsv.h >= 360)
        hsv.h -= 360;

    hsv.s = mx == 0 ? 0 : (mx - mn);
    hsv.v = mx;
    return hsv;
}

// static
double HSV::diff2(const HSV& hsv1, const HSV& hsv2)
{
    double x1 = hsv1.s * cos(hsv1.h);
    double y1 = hsv1.s * sin(hsv1.h);
    double z1 = hsv1.v;

    double x2 = hsv2.s * cos(hsv2.h);
    double y2 = hsv2.s * sin(hsv2.h);
    double z2 = hsv2.v;

    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2);
}

RGB HSV::toRGB() const
{
    if (v < 1e-9)
        return RGB(0, 0, 0);

    int i = static_cast<int>(floor(h / 60.0)) % 6;
    double f = (h / 60.0) - floor(h / 60.0);
    int p = static_cast<int>(round(v * (1.0 - (s / v))));
    int q = static_cast<int>(round(v * (1.0 - (s / v) * f)));
    int t = static_cast<int>(round(v * (1.0 - (s / v) * (1.0 - f))));

    switch(i){
    case 0: return RGB(v, t, p);
    case 1: return RGB(q, v, p);
    case 2: return RGB(p, v, t);
    case 3: return RGB(p, q, v);
    case 4: return RGB(t, p, v);
    case 5: return RGB(v, p, q);
    }

    return RGB(0, 0, 0);
}
