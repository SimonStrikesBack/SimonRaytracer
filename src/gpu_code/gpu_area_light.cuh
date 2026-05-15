/**
 * @file gpu_area_light.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_AreaLight class
 */
#pragma once

#include "gpu_triangle.cuh"
#include "gpu_vec3.cuh"
#include "../area_light.hpp"

/**
 * @class gpu_AreaLight
 *
 * GPU compatible AreaLight class
 */
class gpu_AreaLight {
public:
    gpu_vec3 color;     /**< Light color */
    gpu_triangle face;  /**< Area light's face */
    float area;         /**< Area of the area light */
    gpu_vec3 normal;    /**< Normal vector of the area light */

    /**
     * Constructor from a CPU AreaLight
     *
     * @param[in] a     CPU AreaLight
     */
    __host__
    gpu_AreaLight(const AreaLight& a): color(a.color), face(a.face), area(a.area), normal(a.normal){}
};
