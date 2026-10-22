#pragma once

#include "color.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class Image {
public:
    size_t width;
    size_t height;
    Image(size_t w, size_t h);
    Color& At(size_t x, size_t y);
    const Color& InBorders(int64_t x, int64_t y) const;
    Color GetInterpolated(double x, double y) const;
    void Swap(Image& other);

private:
    std::vector<Color> data_;
};
