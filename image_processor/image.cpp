#include "image.h"
#include "color.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

Image::Image(size_t w, size_t h) {
    if (w == 0 || h == 0) {
        throw std::invalid_argument("width and height need to be not zero");
    }
    width = w;
    height = h;
    data_ = std::vector<Color>(w * h);
}

Color& Image::At(size_t x, size_t y) {
    if (x >= width || y >= height) {
        throw std::out_of_range("wrong coordinates of pixel");
    }
    return data_[y * width + x];
}

const Color& Image::InBorders(int64_t x, int64_t y) const {
    if (x < 0) {
        x = 0;
    } else if (x >= static_cast<int64_t>(width)) {
        x = static_cast<int64_t>(width) - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= static_cast<int64_t>(height)) {
        y = static_cast<int64_t>(height) - 1;
    }
    return data_[static_cast<size_t>(y) * width + static_cast<size_t>(x)];
}

Color Image::GetInterpolated(double x, double y) const {
    if (x < 0.0) {
        x = 0.0;
    } else if (x > static_cast<double>(width - 1)) {
        x = static_cast<double>(width - 1);
    }
    if (y < 0.0) {
        y = 0.0;
    } else if (y > static_cast<double>(height - 1)) {
        y = static_cast<double>(height - 1);
    }
    int64_t left = static_cast<int64_t>(x);
    int64_t top = static_cast<int64_t>(y);
    double fx = x - static_cast<double>(left);
    double fy = y - static_cast<double>(top);
    const Color& a = InBorders(left, top);
    const Color& b = InBorders(left + 1, top);
    const Color& c = InBorders(left, top + 1);
    const Color& d = InBorders(left + 1, top + 1);
    double k1 = (1.0 - fx) * (1.0 - fy);
    double k2 = fx * (1.0 - fy);
    double k3 = (1.0 - fx) * fy;
    double k4 = fx * fy;
    Color res;
    res.red = a.red * k1 + b.red * k2 + c.red * k3 + d.red * k4;
    res.green = a.green * k1 + b.green * k2 + c.green * k3 + d.green * k4;
    res.blue = a.blue * k1 + b.blue * k2 + c.blue * k3 + d.blue * k4;
    return res;
}

void Image::Swap(Image& other) {
    size_t tmp = width;
    width = other.width;
    other.width = tmp;
    tmp = height;
    height = other.height;
    other.height = tmp;
    data_.swap(other.data_);
}
