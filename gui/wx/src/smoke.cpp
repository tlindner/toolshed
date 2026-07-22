#include "disk_image.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    const bool list_mode = argc == 2 || argc == 3;
    const bool extract_mode = argc == 5 && std::string(argv[2]) == "--extract";
    if (!list_mode && !extract_mode) {
        std::cerr << "usage: toolshed-image-smoke IMAGE [DIRECTORY]\n"
                     "       toolshed-image-smoke IMAGE --extract FILE OUTPUT\n";
        return 2;
    }

    try {
        const auto image = toolshed::DiskImage::open(argv[1]);
        std::cout << image.format_name() << "\n";
        if (extract_mode) {
            const auto bytes = image.read_file(argv[3]);
            std::ofstream output(argv[4], std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes.data()),
                         static_cast<std::streamsize>(bytes.size()));
            if (!output) {
                throw std::runtime_error("Could not write extraction output");
            }
            std::cout << "extracted " << bytes.size() << " bytes\n";
            return 0;
        }

        const std::string directory = argc == 3 ? argv[2] : "";
        for (const auto& entry : image.list_directory(directory)) {
            std::cout << (entry.directory ? "d " : "f ") << entry.size << " "
                      << entry.name << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}
