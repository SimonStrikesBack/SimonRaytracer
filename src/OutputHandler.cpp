/**
 * @file OutputHandler.cpp
 * @author Simon Tanev
 * @brief Implementation file for OutputHandler.hpp, see the header file for descriptions
 */

#include "OutputHandler.hpp"
#include <fstream>
#include <cstring>

float *OutputHandler::CreateOutputBuffer(const int width, const int height, const Context::Channels channels) {
    float* ret = new float[width * height * channels];
    return ret;
}

void OutputHandler::WriteOutputBuffer(const int width, const int height, const Context::Channels channels, const char* path, const char *name, const float *data) {
    char full_path[128];
    strcpy(full_path, path);
    strcat(full_path, name);
    std::cout << full_path << std::endl;
    std::ofstream output(full_path);
    output << "P3\n" << width << " " << height << "\n" << 255 << std::endl;
    if (channels == Context::RGB) {
        for (int i = 0; i < width * height * 3; ++i) output << (int)data[i] << " ";
    }
    else for (int i = 0; i < width * height; ++i) {
        output << (int)data[i] << " ";
        output << (int)data[i] << " ";
        output << (int)data[i] << " ";
    }
    output.close();
}
