/**
 * @file area_light.hpp
 * @author Simon Tanev
 * @brief File containing AreaLight class
 */

#ifndef RAYTRACER_AREA_LIGHT_HPP
#define RAYTRACER_AREA_LIGHT_HPP
#pragma once
#include <complex>

#include "light.hpp"
#include "triangle.hpp"
#include "geomath.hpp"
#include "vec3.hpp"

/**
 * @class AreaLight
 * Holds information of an area light triangle (not the whole mesh)
 * Inherits from a base Light class
 */
class AreaLight: public Light {
public:
    triangle face;  /**< Triangle of the area light in scene */
    float area;     /**< Area of the triangle */
    vec3 normal;    /**< Normal vector of the face */

    /**
     * Basic minimal constructor, automatically calculates the area and normal
     * @param[in]   face    face of the to be created light
     * @param[in]   color   color of the to be created light
     */
    AreaLight(const triangle &face, const vec3 color): Light(color), face(face) {
        area = face.area();
        const vec3 a = face.b.position - face.a.position;
        const vec3 b = face.c.position - face.a.position;
        normal = cross(a, b).normalize();
    }

    /**
     * Returns a string representing the type of light
     */
    [[nodiscard]] std::string Type() const override { return "Area"; }
};

#endif //RAYTRACER_AREA_LIGHT_HPP