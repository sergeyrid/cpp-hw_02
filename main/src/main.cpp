#include <iostream>
#include <string>
#include "huffman.h"

int main(int argc, char *argv[]) {
    bool zip = true;
    std::string in_filename;
    std::string out_filename;
    for (std::size_t i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "-u") {
            zip = false;
        } else if (arg == "-c") {
            zip = true;
        } else if ((arg == "-f" || arg == "--file") && i < argc - 1) {
            in_filename = argv[i + 1];
            ++i;
        } else if ((arg == "-o" || arg == "--output") && i < argc - 1) {
            out_filename = argv[i + 1];
            ++i;
        } else {
            std::cerr << "Invalid argument: \"" << arg <<  "\"";
            return 1;
        }
    }
    huffman_algo::HuffmanArchiver archiver(in_filename, out_filename);
    try {
        if (zip) {
            archiver.zip();
        } else {
            archiver.unzip();
        }
        std::cout << archiver.get_in_file_size() << '\n';
        std::cout << archiver.get_out_file_size() << '\n';
        std::cout << archiver.get_extra_data_size();
    } catch (const std::exception &e) {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}
