/**
 * @file light.hpp
 * @author Simon Tanev
 * @brief Header file containing the Light class
 */

#ifndef RAYTRACER_LIGHT_HPP
#define RAYTRACER_LIGHT_HPP
#pragma once
#include <string>
#include "vec3.hpp"

/**
 * @class Light
 * Base class for lights
 */
class Light {
public:
    virtual ~Light() = default;

    vec3 color;     /**< Color of the light */

    /**
     * Basic Constructor that just assigns all the input fields
     *
     * @param[in] color     color of the light
     */
    Light(const vec3 color): color(color){}

    /**
     * Virtual Type method, implementations return the type string
     */
    [[nodiscard]] virtual std::string Type() const = 0;
};


#endif //RAYTRACER_LIGHT_HPP
