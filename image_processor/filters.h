#pragma once

#include "image.h"

#include <cstddef>

const double MAX_BLUR_SIGMA = 1000.0;

class Filter {
protected:
    void ClampColors(Image& image) const;
};

class CropFilter : public Filter {
public:
    CropFilter(size_t w, size_t h);
    void Apply(Image& image) const;

private:
    size_t w_;
    size_t h_;
};

class GrayscaleFilter : public Filter {
public:
    void Apply(Image& image) const;
};

class NegativeFilter : public Filter {
public:
    void Apply(Image& image) const;
};

class CircleSplashFilter : public Filter {
public:
    CircleSplashFilter(double cx, double cy, double r);
    void Apply(Image& image) const;

private:
    double cx_;
    double cy_;
    double r_;
};

class MatrixFilter : public Filter {
public:
    void Apply(Image& image) const;

protected:
    MatrixFilter();
    double matrix_[3][3];
};

class SharpeningFilter : public MatrixFilter {
public:
    SharpeningFilter();
};

class EdgeDetectionFilter : public MatrixFilter {
public:
    explicit EdgeDetectionFilter(double limit);
    void Apply(Image& image) const;

private:
    double limit_;
};

class GaussianBlurFilter : public Filter {
public:
    explicit GaussianBlurFilter(double sigma);
    void Apply(Image& image) const;

private:
    double sigma_;
};
