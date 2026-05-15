/**
 * @file OutputHandler.hpp
 * @author Simon Tanev
 * @brief Header file containing the OutputHandler namespace
 */

#ifndef RAYTRACER_OUTPUTHANDLER_H
#define RAYTRACER_OUTPUTHANDLER_H
#pragma once

#include "context.hpp"

/**
 * @namespace OutputHandler
 *
 * Holds helper functions for output management
 */
namespace OutputHandler {

    /**
     * Returns a pointer to allocated memory for a buffer
     *
     * @param width         width of the buffer
     * @param height        height of the buffer
     * @param channels      channels of the buffer
     */
    float* CreateOutputBuffer(int width, int height, Context::Channels channels);

    /**
     * Writes buffer data to an output ppm file
     *
     * @param width     width of the buffer
     * @param height    height of the buffer
     * @param channels  channels of the buffer
     * @param path      path to the output directory
     * @param name      name of the output file
     * @param data      pointer to the data
     */
    void WriteOutputBuffer(int width, int height, Context::Channels channels, const char* path, const char* name, const float* data);
}


#endif //RAYTRACER_OUTPUTHANDLER_H