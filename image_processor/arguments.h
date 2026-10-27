#pragma once

#include "filters.h"
#include "image.h"

#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct Arguments {
    std::string in;
    std::string out;
    std::vector<std::string> filters;
};

Arguments ParseArguments(const std::vector<std::string>& args);

std::vector<std::unique_ptr<Filter>> CreateFilters(const std::vector<std::string>& args);

void ApplyFilters(const std::vector<std::unique_ptr<Filter>>& filters, Image& img);

void PrintHelp(std::ostream& output);
