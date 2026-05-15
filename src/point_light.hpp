/**
 * @file point_light.hpp
 * @author Simon Tanev
 * @brief File containing PointLight class
 */

#ifndef RAYTRACER_POINT_LIGHT_HPP
#define RAYTRACER_POINT_LIGHT_HPP
#pragma once
#include "light.hpp"

/**
 * @class PointLight
 * Holds information about a point light
 * Inherits from a base Light class
 */
class PointLight : public Light{
public:
    vec3 position;      /**< Position of the point light */

    /**
     * Basic minimal constructor
     *
     * @param[in]   position    position of the to be created light
     * @param[in]   color       color of the to be created light
     */
    PointLight(const vec3 position, const vec3 color): Light(color), position(position){}

    /**
     * Returns a string representing the type of light
     */
    [[nodiscard]] std::string Type() const override { return "Point"; }
};


#endif //RAYTRACER_POINT_LIGHT_HPP