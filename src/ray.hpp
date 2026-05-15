/**
 * @file ray.hpp
 * @author Simon Tanev
 * @brief File containing Ray class
 */
#ifndef RAYTRACER_RAY_HPP
#define RAYTRACER_RAY_HPP
#pragma once
#include "vec3.hpp"

/**
 * @class Ray
 *
 * Represents a ray, from an origin is some direction
 */
class Ray{
public:
    vec3 origin;    /**< Ray origin vector */
    vec3 dir;       /**< Ray direction vector */
    vec3 inv_dir;   /**< Ray inverse direction vector */

    /**
     * Basic Constructor that just assigns all the input fields and calculates the inverse direction
     *
     * @param[in] origin     ray origin
     * @param[in] dir        ray direction
     */
    Ray(const vec3& origin, const vec3& dir): origin(origin), dir(dir.normalize()) {
        inv_dir = 1.0f / dir;
    }
};


#endif //RAYTRACER_RAY_HPP
