/**
 * @file gpu_vertex.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_vertex class
 */

#pragma once

#include "gpu_vec3.cuh"
#include "gpu_vec2.cuh"
#include "../vertex.hpp"

/**
 * @class gpu_vertex
 *
 * GPU compatible vertex class
 */
class gpu_vertex {
public:
    gpu_vec3 position;      /**< Position vector */
    gpu_vec3 normal;        /**< Normal vector */
    gpu_vec2 uv;            /**< UV vector */

    /**
     * Construct a vertex from position
     * Normal and UV are zero initialized
     *
     * @param[in] position      vertex position
     */
    __host__ __device__
    explicit gpu_vertex(const gpu_vec3& position): position(position){ normal = gpu_vec3(0.0f); uv = gpu_vec2(0.0f); }

    /**
     * Construct a vertex from position and normal
     * UV is zero initialized
     *
     * @param[in] position      vertex position
     * @param[in] normal        vertex normal
     */
    __host__ __device__
    explicit gpu_vertex(const gpu_vec3& position, const gpu_vec3& normal): position(position), normal(normal) { uv = gpu_vec2(0.0f); }

    /**
     * Construct a vertex from position, normal and uv
     *
     * @param[in] position      vertex position
     * @param[in] normal        vertex normal
     * @param[in] uv            vertex uv
     */
    __host__ __device__
    explicit gpu_vertex(const gpu_vec3& position, const gpu_vec3& normal, const gpu_vec2& uv): position(position), normal(normal), uv(uv){}

    /**
     * Constructor from a CPU vertex
     *
     * @param[in] v     CPU vertex
     */
    __host__
    explicit gpu_vertex(const vertex& v): position(v.position), normal(v.normal), uv(v.uv) {}
};

