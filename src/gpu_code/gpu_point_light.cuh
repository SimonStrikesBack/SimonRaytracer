/**
 * @file gpu_point_light.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_PointLight class
 */

#pragma once
#include "../point_light.hpp"

/**
 * @class gpu_PointLight
 *
 * GPU compatible PointLight class
 */
class gpu_PointLight {
public:
    gpu_vec3 color;         /**< Light color */
    gpu_vec3 position;      /**< Light position */

    /**
     * Basic class constructor
     *
     * @param[in] position      light position
     * @param[in] color         light color
     */
    __host__ __device__
    gpu_PointLight(const gpu_vec3 position, const gpu_vec3 color): color(color), position(position){}

    /**
     * Constructor from a CPU PointLight
     *
     * @param[in] p     CPU PointLight
     */
    __host__
    gpu_PointLight(const PointLight& p): color(p.color), position(p.position) {}

};

