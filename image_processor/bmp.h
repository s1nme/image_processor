#pragma once

#include "image.h"

#include <string>

Image ReadBmp(const std::string& file);
void WriteBmp(const std::string& file, const Image& img);
