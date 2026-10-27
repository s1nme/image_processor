#include "arguments.h"
#include "bmp.h"
#include "filters.h"
#include "image.h"

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc <= 3) {
        PrintHelp(std::cout);
        return 0;
    }
    try {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        Arguments opts = ParseArguments(args);
        std::vector<std::unique_ptr<Filter>> filters = CreateFilters(opts.filters);
        Image img = ReadBmp(opts.in);
        ApplyFilters(filters, img);
        WriteBmp(opts.out, img);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
