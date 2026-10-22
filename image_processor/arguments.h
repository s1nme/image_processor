#pragma once

#include "image.h"

#include <ostream>
#include <string>
#include <vector>

struct Arguments {
    std::string in;
    std::string out;
    std::vector<std::string> filters;
};

Arguments ParseArguments(const std::vector<std::string>& args);
void ApplyFilters(const std::vector<std::string>& args, Image& img);
void PrintHelp(std::ostream& output);
