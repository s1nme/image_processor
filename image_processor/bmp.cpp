#include "bmp.h"
#include "color.h"
#include "image.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

const uint32_t BMP_SIGNATURE = 0x4d42;
const uint32_t BMP_HEADER_SIZE = 54;
const uint32_t DIB_HEADER_SIZE = 40;
const uint32_t BMP_BITS_PER_PIXEL = 24;
const uint32_t BYTES_PER_PIXEL = 3;
const uint32_t ROW_ALIGNMENT = 4;
const uint32_t SHORT_FIELD_SIZE = 2;
const uint32_t LONG_FIELD_SIZE = 4;
const uint32_t MAX_BYTE = 255;
const uint32_t BYTE_BASE = 256;
const int64_t UINT32_RANGE = 4294967296;

static uint32_t ReadNumber(std::ifstream& fin, uint32_t n) {
    uint32_t res = 0;
    uint32_t power = 1;
    for (uint32_t i = 0; i < n; ++i) {
        char byte = 0;
        fin.get(byte);
        if (!fin) {
            throw std::invalid_argument("can't read number");
        }
        res += static_cast<unsigned char>(byte) * power;
        if (i + 1 < n) {
            power *= BYTE_BASE;
        }
    }
    return res;
}

static uint64_t GetRowSize(size_t w) {
    uint64_t pixel_bytes = static_cast<uint64_t>(w) * BYTES_PER_PIXEL;
    while (pixel_bytes % ROW_ALIGNMENT != 0) {
        ++pixel_bytes;
    }
    return pixel_bytes;
}

Image ReadBmp(const std::string& file) {
    std::ifstream fin(file, std::ios::binary);
    if (!fin) {
        throw std::invalid_argument("wrong input file: " + file);
    }
    fin.seekg(0, std::ios::end);
    std::streamoff size = fin.tellg();
    if (size < BMP_HEADER_SIZE) {
        throw std::invalid_argument("too short for bmp file");
    }
    fin.seekg(0);
    uint32_t signature = ReadNumber(fin, SHORT_FIELD_SIZE);
    if (signature != BMP_SIGNATURE) {
        throw std::invalid_argument("that is not a bmp file");
    }
    uint32_t file_bytes = ReadNumber(fin, LONG_FIELD_SIZE);
    uint32_t reserved = ReadNumber(fin, LONG_FIELD_SIZE);
    uint32_t pixel_data_position = ReadNumber(fin, LONG_FIELD_SIZE);
    uint32_t header_size = ReadNumber(fin, LONG_FIELD_SIZE);
    if (reserved != 0) {
        throw std::invalid_argument("reesrved bmp field need to be zero");
    }
    if (header_size != DIB_HEADER_SIZE) {
        throw std::invalid_argument("it's not a bmp with 40 byte BITMAPINFOHEADER");
    }
    uint32_t w = ReadNumber(fin, LONG_FIELD_SIZE);
    int64_t h = ReadNumber(fin, LONG_FIELD_SIZE);
    if (h > std::numeric_limits<int32_t>::max()) {
        h -= UINT32_RANGE;
    }
    bool from_top = h < 0;
    if (from_top) {
        h = -h;
    }
    if (w == 0 || w > std::numeric_limits<int32_t>::max()) {
        throw std::invalid_argument("wrong bmp width");
    }
    if (h == 0 || h > std::numeric_limits<int32_t>::max()) {
        throw std::invalid_argument("wrong bmp height");
    }
    uint32_t planes = ReadNumber(fin, SHORT_FIELD_SIZE);
    uint32_t bits_per_pixel = ReadNumber(fin, SHORT_FIELD_SIZE);
    uint32_t compression_type = ReadNumber(fin, LONG_FIELD_SIZE);
    uint32_t data_size = ReadNumber(fin, LONG_FIELD_SIZE);
    ReadNumber(fin, LONG_FIELD_SIZE);
    ReadNumber(fin, LONG_FIELD_SIZE);
    uint32_t color_count = ReadNumber(fin, LONG_FIELD_SIZE);
    ReadNumber(fin, LONG_FIELD_SIZE);
    if (planes != 1 || bits_per_pixel != BMP_BITS_PER_PIXEL || compression_type != 0 || color_count != 0) {
        throw std::invalid_argument("wrong type of bmp");
    }
    uint64_t row_bytes = GetRowSize(w);
    uint64_t data_bytes = row_bytes * static_cast<uint64_t>(h);
    uint64_t need = pixel_data_position + data_bytes;
    if (pixel_data_position < BMP_HEADER_SIZE) {
        throw std::invalid_argument("pixels over header");
    }
    if (need > static_cast<uint64_t>(size)) {
        throw std::invalid_argument("wrong pixel data");
    }
    if (file_bytes < need || file_bytes > static_cast<uint64_t>(size)) {
        throw std::invalid_argument("wrong bmp file size in header");
    }
    if (data_size != 0 && data_size != data_bytes) {
        throw std::invalid_argument("wrong pixel data size in header");
    }
    Image img(w, static_cast<size_t>(h));
    uint64_t add_pixels = row_bytes - static_cast<uint64_t>(w) * BYTES_PER_PIXEL;
    fin.seekg(pixel_data_position);
    for (size_t i = 0; i < img.height; ++i) {
        size_t y = i;
        if (!from_top) {
            y = img.height - 1 - i;
        }
        for (size_t x = 0; x < img.width; ++x) {
            char pixel[BYTES_PER_PIXEL] = {};
            fin.read(pixel, BYTES_PER_PIXEL);
            if (!fin) {
                throw std::invalid_argument("can't read pixel in bmp");
            }
            Color& c = img.At(x, y);
            c.blue = static_cast<unsigned char>(pixel[0]) / static_cast<double>(MAX_BYTE);
            c.green = static_cast<unsigned char>(pixel[1]) / static_cast<double>(MAX_BYTE);
            c.red = static_cast<unsigned char>(pixel[2]) / static_cast<double>(MAX_BYTE);
        }
        fin.ignore(static_cast<std::streamsize>(add_pixels));
        if (!fin) {
            throw std::invalid_argument("can't read row of pixels in bmp");
        }
    }
    return img;
}

static void WriteNumber(std::ofstream& fout, uint32_t value, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) {
        fout.put(static_cast<char>(value % BYTE_BASE));
        value /= BYTE_BASE;
    }
}

void WriteBmp(const std::string& file, const Image& img) {
    uint64_t row_bytes = GetRowSize(img.width);
    uint64_t data_size = row_bytes * img.height;
    uint64_t file_bytes = BMP_HEADER_SIZE + data_size;
    std::ofstream fout(file, std::ios::binary);
    if (!fout) {
        throw std::invalid_argument("wrong output file: " + file);
    }
    WriteNumber(fout, BMP_SIGNATURE, SHORT_FIELD_SIZE);
    WriteNumber(fout, static_cast<uint32_t>(file_bytes), LONG_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    WriteNumber(fout, BMP_HEADER_SIZE, LONG_FIELD_SIZE);
    WriteNumber(fout, DIB_HEADER_SIZE, LONG_FIELD_SIZE);
    WriteNumber(fout, static_cast<uint32_t>(img.width), LONG_FIELD_SIZE);
    WriteNumber(fout, static_cast<uint32_t>(img.height), LONG_FIELD_SIZE);
    WriteNumber(fout, 1, SHORT_FIELD_SIZE);
    WriteNumber(fout, BMP_BITS_PER_PIXEL, SHORT_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    WriteNumber(fout, static_cast<uint32_t>(data_size), LONG_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    WriteNumber(fout, 0, LONG_FIELD_SIZE);
    uint64_t add_pixels = row_bytes - img.width * BYTES_PER_PIXEL;
    for (size_t i = 0; i < img.height; ++i) {
        size_t y = img.height - 1 - i;
        for (size_t x = 0; x < img.width; ++x) {
            Color c = img.InBorders(static_cast<int64_t>(x), static_cast<int64_t>(y));
            c.Clamp();
            char pixel[BYTES_PER_PIXEL] = {};
            pixel[0] = static_cast<char>(std::lround(c.blue * MAX_BYTE));
            pixel[1] = static_cast<char>(std::lround(c.green * MAX_BYTE));
            pixel[2] = static_cast<char>(std::lround(c.red * MAX_BYTE));
            fout.write(pixel, BYTES_PER_PIXEL);
        }
        for (uint64_t j = 0; j < add_pixels; ++j) {
            fout.put(0);
        }
    }
    fout.close();
    if (!fout) {
        throw std::invalid_argument("there is problem in writing to output file");
    }
}
