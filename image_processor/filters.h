#pragma once

#include "color.h"
#include "image.h"

#include <cstddef>
#include <string>

const double MAX_BLUR_SIGMA = 1000.0;

class Filter {
public:
    virtual ~Filter() = default;

    virtual void Apply(Image& image) const = 0;

protected:
    void ClampColors(Image& image) const;
};

class CropFilter : public Filter {
public:
    CropFilter(size_t w, size_t h);

    void Apply(Image& image) const override;

private:
    size_t w_;
    size_t h_;
};

class PixelFilter : public Filter {
public:
    void Apply(Image& image) const override;

protected:
    virtual void ChangeColor(Color& color) const = 0;
};

class GrayscaleFilter : public PixelFilter {
protected:
    void ChangeColor(Color& color) const override;
};

class NegativeFilter : public PixelFilter {
protected:
    void ChangeColor(Color& color) const override;
};

class MatrixFilter : public Filter {
public:
    void Apply(Image& image) const override;

protected:
    MatrixFilter();
    double matrix_[3][3];
};

class SharpFilter : public MatrixFilter {
public:
    SharpFilter();
};

class EdgeDetectionFilter : public MatrixFilter {
public:
    explicit EdgeDetectionFilter(double limit);

    void Apply(Image& image) const override;

private:
    double limit_;
};

class GaussianBlurFilter : public Filter {
public:
    explicit GaussianBlurFilter(double sigma);

    void Apply(Image& image) const override;

private:
    double sigma_;
};

class CircleSplashFilter : public Filter {
public:
    CircleSplashFilter(double cx, double cy, double r);

    void Apply(Image& image) const override;

private:
    double cx_;
    double cy_;
    double r_;
};

class CropPositionFilter : public Filter {
public:
    CropPositionFilter(size_t w, size_t h, std::string pos);

    void Apply(Image& image) const override;

private:
    std::string pos_;
    size_t w_;
    size_t h_;
};
