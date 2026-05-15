/**
 * @file gpu_texture.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Texture class
 */


#pragma once
#include "../include/lodepng.h"
#include "gpu_vec3.cuh"
#include "gpu_vec2.cuh"
#include "gpu_geomath.cuh"
#include <cmath>
#include "../texture.hpp"

/**
 * @class gpu_Texture
 *
 * GPU compatible Texture class
 */
class gpu_Texture {
public:
    unsigned int width;         /**< Texture width */
    unsigned int height;        /**< Texture height */

    unsigned char* data;        /**< Raw data */
    size_t data_size;           /**< Data size */

    /**
     * Destructor of the class
     * It calls free functions on the CUDA memory stored
     */
    __host__ __device__
    ~gpu_Texture() {
        if (data != nullptr) cudaFree(data);
    }

    /**
     * Constructor from a CPU Texture
     *
     * @param[in] t     CPU Texture
     */
    __host__
    gpu_Texture(const Texture& t) : width(t.width), height(t.height), data(nullptr), data_size(t.data_size) {
        cudaMallocManaged(&data, t.data_size);
        memcpy(data, t.data, t.data_size);
    }

    /**
     * Returns the texture sample at the exact integer uv coordinates clamped to [0,1]
     *
     * @param[in] u     u coordinate
     * @param[in] v     v coordinate
     */
    __host__ __device__
    gpu_vec3 at(const unsigned int u, const unsigned int v) const {
        if (u >= width) return {0.0f};
        if (v >= height) return {0.0f};

        const float r = static_cast<float>(data[4 * width * v + 4 * u + 0]) / 255.0f;
        const float g = static_cast<float>(data[4 * width * v + 4 * u + 1]) / 255.0f;
        const float b = static_cast<float>(data[4 * width * v + 4 * u + 2]) / 255.0f;

        return gpu_vec3(gpu_clamp(r, 0.0f, 1.0f), gpu_clamp(g, 0.0f, 1.0f), gpu_clamp(b, 0.0f, 1.0f));
    }

    /**
     * Samples the texture at a general uv position (in range [0,1], both u and v are not whole numbers)
     * Uses bilinear filtration to interpolate based on u and v
     *
     * @param[in] u     u coordinate
     * @param[in] v     v coordinate
     * @param[in] u1    floor of u
     * @param[in] u2    ceil of u
     * @param[in] v1    floor of v
     * @param[in] v2    ceil of v
     */
    __host__ __device__
    gpu_vec3 bilinear_sample_xy(const float u, const float v, const unsigned int u1, const unsigned int u2, const unsigned int v1,const unsigned int v2 ) const {
        const gpu_vec3 top_left = at(u1, v2);
        const gpu_vec3 top_right = at(u2, v2);
        const gpu_vec3 bottom_left = at(u1,v1);
        const gpu_vec3 bottom_right = at(u2, v1);

        const float top_left_mult = ((static_cast<float>(u2) - u) * (v - static_cast<float>(v1)))/static_cast<float>((u2 - u1) * (v2 - v1));
        const float top_right_mult = ((u - static_cast<float>(u1))*(v - static_cast<float>(v1)))/static_cast<float>((u2 - u1) * (v2 - v1));
        const float bottom_left_mult = ((static_cast<float>(u2) - u)*(static_cast<float>(v2) - v))/static_cast<float>((u2 - u1) * (v2 - v1));
        const float bottom_right_mult = ((u - static_cast<float>(u1))*(static_cast<float>(v2) - v))/static_cast<float>((u2 - u1) * (v2 - v1));

        return gpu_vec3(top_left * top_left_mult + top_right * top_right_mult + bottom_left * bottom_left_mult + bottom_right * bottom_right_mult);
    }

    /**
     * Samples the texture at general u position (u is not a whole number) and a whole number v position (u and v in range [0,1])
     * Uses linear filtration to interpolate based on u
     *
     * @param[in] u     u coordinate
     * @param[in] u1    floor of u
     * @param[in] u2    ceil of u
     * @param[in] v     whole number v coordinate
     */
    __host__ __device__
    gpu_vec3 bilinear_sample_x(const float u, const unsigned int u1, const unsigned int u2, const unsigned int v) const {
        const gpu_vec3 right = at(u2, v);
        const gpu_vec3 left = at(u1, v);

        const float right_mult = static_cast<float>(u2) - u;
        const float left_mult = u - static_cast<float>(u1);

        return gpu_vec3(right * right_mult + left * left_mult);
    }

    /**
     * Samples the texture at general v position (v is not a whole number) and a whole number u position (u and v in range [0,1])
     * Uses linear filtration to interpolate based on v
     *
     * @param[in] v     v coordinate
     * @param[in] v1    floor of v
     * @param[in] v2    ceil of v
     * @param[in] u     whole number u coordinate
     */
    __host__ __device__
    gpu_vec3 bilinear_sample_y(const unsigned int u, const float v, const unsigned int v1, const unsigned int v2) const {
        const gpu_vec3 top = at(u, v2);
        const gpu_vec3 bottom = at(u, v1);

        const float top_mult = static_cast<float>(v2) - v;
        const float bottom_mult = v - static_cast<float>(v1);

        return gpu_vec3(top * top_mult + bottom * bottom_mult);
    }

    /**
     * Samples the texture based on the texture coordinate
     * It is a wrapper around the other sampling functions and dispatches them based on the u and v values
     *
     * @param[in] tex_coord     input uv coordinates
     */
    __host__ __device__
    gpu_vec3 sample_uv(const gpu_vec2 tex_coord) const {
        const gpu_vec2 uv = tex_coord % 1.0f;
        const float u = uv.x * static_cast<float>(width-1);
        const float v = (uv.y) * static_cast<float>(height-1); // uvs and raw data are flipped vertically

        const unsigned int u1 = floor(u);
        const unsigned int u2 = ceil(u);
        const unsigned int v1 = floor(v);
        const unsigned int v2 = ceil(v);

        if (u1 == u2 && v1 == v2) { // exactly on the pixel
            return at(u1, v2);
        }
        if (u1 == u2) {
            return bilinear_sample_y(u1, v, v1, v2);
        }
        if (v1 == v2) {
            return bilinear_sample_x(u, u1, u2, v1);
        }
        return bilinear_sample_xy(u, v, u1, u2, v1, v2);
    }

    /**
     * Samples the texture based on the texture coordinate and treats the sample as a normal vector sample
     * It is a wrapper around the other sampling functions and dispatches them based on the u and v values
     * It then maps the sample to [-1,1]
     *
     * @param[in] tex_coord     input uv coordinates
     */
    __host__ __device__
    gpu_vec3 sample_uv_normal(const gpu_vec2 tex_coord) const {
        const gpu_vec2 uv = tex_coord % 1.0f;
        const float u = uv.x * static_cast<float>(width-1);
        const float v = (uv.y) * static_cast<float>(height-1); // uvs and raw data are flipped vertically

        const unsigned int u1 = floor(u);
        const unsigned int u2 = ceil(u);
        const unsigned int v1 = floor(v);
        const unsigned int v2 = ceil(v);

        gpu_vec3 normal;
        if (u1 == u2 && v1 == v2) { // exactly on the pixel
            normal = at(u1, v2);
        }
        else if (u1 == u2) {
            normal = bilinear_sample_y(u1, v, v1, v2);
        }
        else if (v1 == v2) {
            normal = bilinear_sample_x(u, u1, u2, v1);
        }
        else normal = bilinear_sample_xy(u, v, u1, u2, v1, v2);

        normal = normal * 2.0f - 1.0f;
        return normal.normalize();
    }

};

