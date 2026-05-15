/**
 * @file gpu_ray.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Ray class
 */

#pragma once
#include "gpu_vec3.cuh"

/**
 * @class gpu_Ray
 *
 * GPU compatible Ray class
 */
class gpu_Ray {
public:
    gpu_vec3 origin;    /**< Ray origin vector */
    gpu_vec3 dir;       /**< Ray direction vector */
    gpu_vec3 inv_dir;   /**< Ray inverse direction vector */

    /**
     * Basic Constructor that just assigns all the input fields and calculates the inverse direction
     *
     * @param[in] origin     ray origin
     * @param[in] dir        ray direction
     */
    __host__ __device__
    gpu_Ray(const gpu_vec3& origin, const gpu_vec3& dir): origin(origin), dir(dir.normalize()) {
        inv_dir = 1.0f / dir;
    }
};


