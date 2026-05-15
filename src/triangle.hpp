/**
 * @file triangle.hpp
 * @author Simon Tanev
 * @brief File containing triangle class
 */

#ifndef RAYTRACER_TRIANGLE_HPP
#define RAYTRACER_TRIANGLE_HPP
#pragma once
#include "vertex.hpp"
#include "ray.hpp"
#include "material.hpp"
#include <memory>

/**
 * @class triangle
 *
 * Represents a triangle geometry, its three vertices and its material
 */
class triangle {
public:
    vertex a;   /**< Vertex A */
    vertex b;   /**< Vertex B */
    vertex c;   /**< Vertex C */
    std::shared_ptr<Material> material;     /**< Triangle's material */

    /**
     *  Basic Constructor that just assigns all the input fields
     *
     * @param[in] a             vertex a
     * @param[in] b             vertex b
     * @param[in] c             vertex c
     * @param[in] material      triangle material
     */
    triangle(const vertex &a, const vertex &b, const vertex &c, const std::shared_ptr<Material> &material): a(a), b(b), c(c), material(material) {}

    /**
     * Basic zero initialize constructor
     */
    triangle(): a({0.0}), b({0.0}), c({0.0}), material(nullptr){}

    /**
     * Calculates and returns the triangle area
     */
    float area() const;

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
    float intersect(const Ray & ray, bool cullback,  float & b1, float & b2, const float epsilon = bigger_epsilon) const;
};


#endif //RAYTRACER_TRIANGLE_HPP
