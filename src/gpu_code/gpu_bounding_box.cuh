/**
 * @file gpu_bounding_box.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_BoundingBox class
 */

#pragma once

#include <memory>
#include "gpu_triangle.cuh"
#include "gpu_ray.cuh"
#include <algorithm>

#include "../bounding_box.hpp"

/**
 * @class gpu_BoundingBox
 *
 * GPU compatible BoundingBox class
 */
class gpu_BoundingBox {
public:
    gpu_triangle face;      /**< The face contained in the bounding box */
    bool has_face;          /**< Whether BB contains a valid face value (would be nullptr in CPU class) */
    gpu_vec3 min;           /**< One of the corners of the bounding box (minimal values) */
    gpu_vec3 max;           /**< One of the corners of the bounding box (maximal values) */
    gpu_vec3 centroid;      /**< Centroid of the bounding box */
    float surface_area;     /**< Surface Area of the bounding box */

    // Buy more VRAM if you can not afford to have two face instances per face

    /**
     * Constructor from a CPU BoundingBox
     *
     * @param[in] b     CPU BoundingBox
     */
    __host__
    gpu_BoundingBox(const BoundingBox& b): min(b.min), max(b.max), centroid(b.centroid), surface_area(b.surface_area) {
        if (b.face != nullptr) {face = gpu_triangle(*b.face); has_face = true;}
        else {face = gpu_triangle(); has_face = false;}
    }

    /**
     * Calculates and returns the centroid of the bounding box based on its current min and max values
     */
    __host__ __device__
    gpu_vec3 Centroid() const;

    /**
     * Calculates and returns the surface area of the bounding box based on its current min and max values
     */
    __host__ __device__
    float SurfaceArea() const;

    /**
     * Calculates and returns the t scaling value to an intersection of the ray with the bounding box
     * Returns infinity if there is no intersection, otherwise returns the closest intersection's t scale value
     *
     * @param[in]   ray     The ray to calculate the intersection with
     */
    __host__ __device__
    float RayIntersection(const gpu_Ray& ray) const;
};

