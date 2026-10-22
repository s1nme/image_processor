#include "color.h"

#include <cmath>
#include <stdexcept>

Color::Color() {
    red = 0.0;
    green = 0.0;
    blue = 0.0;
}

Color::Color(double r, double g, double b) {
    red = r;
    green = g;
    blue = b;
}

void Color::Clamp() {
    if (!std::isfinite(red) || !std::isfinite(green) || !std::isfinite(blue)) {
        throw std::runtime_error("color components need to be finite");
    }
    if (red < 0.0) {
        red = 0.0;
    }
    if (red > 1.0) {
        red = 1.0;
    }
    if (green < 0.0) {
        green = 0.0;
    }
    if (green > 1.0) {
        green = 1.0;
    }
    if (blue < 0.0) {
        blue = 0.0;
    }
    if (blue > 1.0) {
        blue = 1.0;
    }
}
