/**
 * @file gpu_triangle.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_triangle class
 */
#pragma once

#include "gpu_vertex.cuh"
#include "gpu_ray.cuh"
#include "gpu_material.cuh"
#include "../triangle.hpp"

/**
 * @class gpu_triangle
 *
 * GPU compatible triangle class
 */
class gpu_triangle {
public:
    gpu_vertex a;               /**< Vertex A */
    gpu_vertex b;               /**< Vertex B */
    gpu_vertex c;               /**< Vertex C */
    gpu_Material* material;     /**< Triangle's material */

    /**
     * Empty zero initializing constructor
     */
    __host__ __device__
    gpu_triangle() : a(gpu_vertex(gpu_vec3(0.0f))), b(gpu_vertex(gpu_vec3(0.0f))), c(gpu_vertex(gpu_vec3(0.0f))) {
        material = nullptr;
    }

    /**
     * Basic class constructor
     *
     * @param[in] a             vertex a
     * @param[in] b             vertex b
     * @param[in] c             vertex c
     * @param[in] material      triangle's material
     */
    __host__ __device__
    gpu_triangle(const gpu_vertex &a, const gpu_vertex &b, const gpu_vertex &c, gpu_Material* material): a(a), b(b), c(c), material(material) {}


    /**
     * Calculates and returns the triangle area
     */
    __host__ __device__
    float area() const;

    /**
     * Constructor from a CPU triangle
     *
     * @param[in] t     CPU triangle
     */
    __host__
    gpu_triangle(const triangle& t): a(t.a), b(t.b), c(t.c){material = nullptr;}

    /**
     * Calculates the intersection point of a ray with the triangle
     * Also passes two of the three barycentric coordinates of the intersection point via b1 and b2
     * It returns the t scaling factor of the ray to the intersection point
     * If it is equal to infinity, then there is no intersection
     *
     * @param[in] ray           ray to test intersection with
     * @param[in] cullback      backface culling toggle
     * @param[out] b1           first barycentric coordinate
     * @param[out] b2           second barycentric coordinate
     * @param[in] epsilon       epsilon for floating point shenanigans
     */
    __host__ __device__
    float intersect(const gpu_Ray & ray, bool cullback,  float & b1, float & b2, const float epsilon = bigger_epsilon) const;
};


