#include "arguments.h"
#include "filters.h"
#include "image.h"

#include <cstddef>
#include <exception>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

const size_t DECIMAL_BASE = 10;

Arguments ParseArguments(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw std::invalid_argument("check what you need to input");
    }
    if (args[0].empty() || args[1].empty()) {
        throw std::invalid_argument("input and output file names can't be empty");
    }
    Arguments res;
    res.in = args[0];
    res.out = args[1];
    for (size_t i = 2; i < args.size(); ++i) {
        res.filters.push_back(args[i]);
    }
    return res;
}

static void CheckCount(const std::vector<std::string>& args, size_t pos, size_t count) {
    if (args.size() - pos - 1 < count) {
        throw std::invalid_argument("wrong count of parametrs " + args[pos]);
    }
}

static size_t ParseNumber(const std::string& text, const std::string& name) {
    size_t value = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            throw std::invalid_argument(name + " need to be a number: " + text);
        }
        size_t digit = static_cast<size_t>(ch - '0');
        if (value > (std::numeric_limits<size_t>::max() - digit) / DECIMAL_BASE) {
            throw std::invalid_argument(name + " is too large: " + text);
        }
        value = value * DECIMAL_BASE + digit;
    }
    if (value == 0) {
        throw std::invalid_argument(name + " need to be greater than zero");
    }
    return value;
}

static double ParseReal(const std::string& text, const std::string& name) {
    size_t characters_read = 0;
    double number = 0.0;
    try {
        number = std::stod(text, &characters_read);
    } catch (const std::exception&) {
        throw std::invalid_argument(name + " need to be a number: " + text);
    }
    if (characters_read != text.size()) {
        throw std::invalid_argument(name + " have wrong arg " + text);
    }
    return number;
}

std::vector<std::unique_ptr<Filter>> CreateFilters(const std::vector<std::string>& args) {
    std::vector<std::unique_ptr<Filter>> filters;
    size_t i = 0;
    while (i < args.size()) {
        if (args[i] == "-crop") {
            CheckCount(args, i, 2);
            size_t w = ParseNumber(args[i + 1], "crop width");
            size_t h = ParseNumber(args[i + 2], "crop height");
            filters.push_back(std::make_unique<CropFilter>(w, h));
            i += 3;
        } else if (args[i] == "-gs") {
            filters.push_back(std::make_unique<GrayscaleFilter>());
            ++i;
        } else if (args[i] == "-neg") {
            filters.push_back(std::make_unique<NegativeFilter>());
            ++i;
        } else if (args[i] == "-sharp") {
            filters.push_back(std::make_unique<SharpFilter>());
            ++i;
        } else if (args[i] == "-edge") {
            CheckCount(args, i, 1);
            double limit = ParseReal(args[i + 1], "edge threshold");
            filters.push_back(std::make_unique<EdgeDetectionFilter>(limit));
            i += 2;
        } else if (args[i] == "-blur") {
            CheckCount(args, i, 1);
            double sigma = ParseReal(args[i + 1], "blur sigma");
            filters.push_back(std::make_unique<GaussianBlurFilter>(sigma));
            i += 2;
        } else if (args[i] == "-circle-splash") {
            CheckCount(args, i, 3);
            double cx = ParseReal(args[i + 1], "circle center x");
            double cy = ParseReal(args[i + 2], "circle center y");
            double r = ParseReal(args[i + 3], "circle radius");
            filters.push_back(std::make_unique<CircleSplashFilter>(cx, cy, r));
            i += 4;
        } else if (args[i] == "-crop_position") {
            CheckCount(args, i, 3);
            size_t w = ParseNumber(args[i + 1], "crop width");
            size_t h = ParseNumber(args[i + 2], "crop height");
            filters.push_back(std::make_unique<CropPositionFilter>(w, h, args[i + 3]));
            i += 4;
        } else {
            throw std::invalid_argument("we dont have this filter :(     : " + args[i]);
        }
    }
    return filters;
}

void ApplyFilters(const std::vector<std::unique_ptr<Filter>>& filters, Image& img) {
    for (const std::unique_ptr<Filter>& filter : filters) {
        filter->Apply(img);
    }
}

void PrintHelp(std::ostream& output) {  // text was made by AI
    output << "Usage: image_processor input.bmp output.bmp [filters...]\n";
    output << "Input: uncompressed 24-bit BMP with a 40-byte BITMAPINFOHEADER.\n";
    output << "Filters are applied from left to right.\n\n";
    output << "-crop width height    Keep the upper-left part; sizes must be positive numbers.\n";
    output << "-gs                   Grayscale\n";
    output << "-neg                  Negative\n";
    output << "-sharp                Sharpen\n";
    output << "-edge threshold       Edge detection; 0 <= threshold <= 1\n";
    output << "-blur sigma           Gaussian blur; 0 < sigma <= 1000\n";
    output << "-circle-splash x y r   Circle splash; radius r must be positive.\n";
    output << "                      x and y are pixels from the upper-left corner.\n\n";
    output << "Without filters, the image pixels are preserved.\n";
    output << "Example: ./image_processor input.bmp output.bmp -circle-splash 150 150 100\n";
}
