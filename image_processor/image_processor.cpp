#include "arguments.h"
#include "bmp.h"
#include "image.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 4) {
        PrintHelp(std::cout);
        return 0;
    }
    try {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        Arguments opts = ParseArguments(args);
        Image img = ReadBmp(opts.in);
        ApplyFilters(opts.filters, img);
        WriteBmp(opts.out, img);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
