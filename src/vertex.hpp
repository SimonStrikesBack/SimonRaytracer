/**
 * @file vertex.hpp
 * @author Simon Tanev
 * @brief File containing vertex class
 */

#ifndef RAYTRACER_VERTEX_HPP
#define RAYTRACER_VERTEX_HPP
#pragma once
#include "vec3.hpp"
#include "vec2.hpp"

/**
 * @class vertex
 *
 * Represents a vertex of a triangle
 */
class vertex {
public:
    vec3 position;      /**< Position vector */
    vec3 normal;        /**< Normal vector */
    vec2 uv;            /**< UV vector */

    /**
     * Construct a vertex from position
     * Normal and UV are zero initialized
     *
     * @param[in] position      vertex position
     */
    explicit vertex(const vec3& position): position(position){ normal = vec3(0.0f); uv = vec2(0.0f); }

    /**
     * Construct a vertex from position and normal
     * UV is zero initialized
     *
     * @param[in] position      vertex position
     * @param[in] normal        vertex normal
     */
    explicit vertex(const vec3& position, const vec3& normal): position(position), normal(normal) { uv = vec2(0.0f); }

    /**
     * Construct a vertex from position, normal and uv
     *
     * @param[in] position      vertex position
     * @param[in] normal        vertex normal
     * @param[in] uv            vertex uv
     */
    explicit vertex(const vec3& position, const vec3& normal, const vec2& uv): position(position), normal(normal), uv(uv){}
};


#endif //RAYTRACER_VERTEX_HPP
