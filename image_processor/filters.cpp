#include "filters.h"

#include "color.h"
#include "image.h"

const double RED_WEIGHT = 0.299;
const double GREEN_WEIGHT = 0.587;
const double BLUE_WEIGHT = 0.114;
const double SHARP_CENTER_WEIGHT = 5.0;
const double EDGE_CENTER_WEIGHT = 4.0;
const double GAUSSIAN_RADIUS_FACTOR = 3.0;
const double GAUSSIAN_DENOMINATOR = 2.0;

void Filter::ClampColors(Image& image) const {
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            image.At(x, y).Clamp();
        }
    }
}

CropFilter::CropFilter(size_t w, size_t h) {
    if (w <= 0 || h <= 0) {
        throw std::invalid_argument("width and height need to be positive");
    }
    w_ = w;
    h_ = h;
}

void CropFilter::Apply(Image& image) const {
    size_t w = w_;
    size_t h = h_;
    if (w > image.width) {
        w = image.width;
    }
    if (h > image.height) {
        h = image.height;
    }
    if (w == image.width && h == image.height) {
        return;
    }
    Image res(w, h);
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            res.At(x, y) = image.At(x, y);
        }
    }
    image.Swap(res);
}

void GrayscaleFilter::Apply(Image& image) const {
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            Color& c = image.At(x, y);
            double gray = RED_WEIGHT * c.red + GREEN_WEIGHT * c.green + BLUE_WEIGHT * c.blue;
            c.red = gray;
            c.green = gray;
            c.blue = gray;
        }
    }
}

void NegativeFilter::Apply(Image& image) const {
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            Color& c = image.At(x, y);
            c.red = 1.0 - c.red;
            c.green = 1.0 - c.green;
            c.blue = 1.0 - c.blue;
        }
    }
}

MatrixFilter::MatrixFilter() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            matrix_[y][x] = 0.0;
        }
    }
}

void MatrixFilter::Apply(Image& image) const {
    Image res(image.width, image.height);
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            Color c;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const Color& p = image.InBorders(static_cast<int64_t>(x) + dx, static_cast<int64_t>(y) + dy);
                    double k = matrix_[dy + 1][dx + 1];
                    c.red += p.red * k;
                    c.green += p.green * k;
                    c.blue += p.blue * k;
                }
            }
            res.At(x, y) = c;
        }
    }
    image.Swap(res);
}

SharpeningFilter::SharpeningFilter() {
    matrix_[0][1] = -1.0;
    matrix_[1][0] = -1.0;
    matrix_[1][1] = SHARP_CENTER_WEIGHT;
    matrix_[1][2] = -1.0;
    matrix_[2][1] = -1.0;
}

EdgeDetectionFilter::EdgeDetectionFilter(double limit) {
    if (limit < 0.0 || limit > 1.0) {
        throw std::invalid_argument("threshold need to be in [0,1]");
    }
    limit_ = limit;
    matrix_[0][1] = -1.0;
    matrix_[1][0] = -1.0;
    matrix_[1][1] = EDGE_CENTER_WEIGHT;
    matrix_[1][2] = -1.0;
    matrix_[2][1] = -1.0;
}

void EdgeDetectionFilter::Apply(Image& image) const {
    GrayscaleFilter gs;
    gs.Apply(image);
    MatrixFilter::Apply(image);
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            double value = 0.0;
            if (image.At(x, y).red > limit_) {
                value = 1.0;
            }
            Color& c = image.At(x, y);
            c.red = value;
            c.green = value;
            c.blue = value;
        }
    }
}

GaussianBlurFilter::GaussianBlurFilter(double sigma) {
    if (sigma <= 0.0 || sigma > MAX_BLUR_SIGMA) {
        throw std::invalid_argument("blur sigma need to be in [0,1000]");
    }
    sigma_ = sigma;
}

void GaussianBlurFilter::Apply(Image& image) const {
    int r = static_cast<int>(std::ceil(GAUSSIAN_RADIUS_FACTOR * sigma_));
    std::vector<double> weights(2 * r + 1);
    double sum = 0.0;
    for (int offset = -r; offset <= r; ++offset) {
        double dist = static_cast<double>(offset) / sigma_;
        double k = std::exp(-dist * dist / GAUSSIAN_DENOMINATOR);
        weights[offset + r] = k;
        sum += k;
    }
    for (double& k : weights) {
        k /= sum;
    }
    Image tmp(image.width, image.height);
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            Color c;
            for (int dx = -r; dx <= r; ++dx) {
                int64_t xx = static_cast<int64_t>(x) + dx;
                const Color& p = image.InBorders(xx, static_cast<int64_t>(y));
                double k = weights[dx + r];
                c.red += p.red * k;
                c.green += p.green * k;
                c.blue += p.blue * k;
            }
            tmp.At(x, y) = c;
        }
    }
    image.Swap(tmp);
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            Color c;
            for (int dy = -r; dy <= r; ++dy) {
                int64_t yy = static_cast<int64_t>(y) + dy;
                const Color& p = image.InBorders(static_cast<int64_t>(x), yy);
                double k = weights[dy + r];
                c.red += p.red * k;
                c.green += p.green * k;
                c.blue += p.blue * k;
            }
            tmp.At(x, y) = c;
        }
    }
    image.Swap(tmp);
}

CircleSplashFilter::CircleSplashFilter(double cx, double cy, double r) {
    if (r <= 0.0) {
        throw std::invalid_argument("numbers can't be negative");
    }
    cx_ = cx;
    cy_ = cy;
    r_ = r;
}

void CircleSplashFilter::Apply(Image& image) const {
    Image res(image.width, image.height);
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            double dx = static_cast<double>(x) - cx_;
            double dy = static_cast<double>(y) - cy_;
            double dist = std::hypot(dx, dy);
            if (dist <= r_) {
                res.At(x, y) = image.At(x, y);
            } else {
                double k = r_ / dist;
                double xx = cx_ + dx * k;
                double yy = cy_ + dy * k;
                res.At(x, y) = image.GetInterpolated(xx, yy);
            }
        }
    }
    image.Swap(res);
}
