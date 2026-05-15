/**
 * @file vec2.hpp
 * @author Simon Tanev
 * @brief File containing vec2 struct
 */

#ifndef RAYTRACER_VEC2_HPP
#define RAYTRACER_VEC2_HPP
#pragma once
#include <iostream>
#include "constants.hpp"
#include <vector>
#include <cassert>
#include <cmath>

struct vec3;

/**
 * @struct vec2
 *
 * Represents a 2-dimensional vector
 */
struct vec2 {
    float x, y;     /**< X and Y coordinates */

    /**
     * Constructs a vec2 from a vec3 by cutting off its z component
     *
     * @param[in] v     vec3 to construct from
     */
    vec2(const vec3& v);

    /**
     * Basic zero initializer constructor
     */
    vec2() : x(0), y(0) {}

    /**
     * Constructs a vec2 from a single scalar value
     *
     * @param[in] v     value to populate x and y
     */
    vec2(const float v) : x(v), y(v) {}

    /**
     * Constructs a vec2 from a specified x and y values
     *
     * @param[in] x     x value
     * @param[in] y     y value
     */
    vec2(const float x, const float y) : x(x), y(y) {}

    /**
     * Constructs a vec2 from a static 3 value array, uses the first two values as x and y
     *
     * @param[in] vector    static array of size 3 to get values from
     */
    vec2(float vector[3]): x(vector[0]), y(vector[1]) {}

    /**
     * Constructs a vec2 from a c++ vector of floats
     * It also asserts that there are enough values to construct a vec2
     *
     * @param[in] v     vector to get values from
     */
    vec2(const std::vector<float> &v){
        assert(v.size() > 1);
        x = v[0];
        y = v[1];
    }

    /**
     * Scalar multiplication
     *
     * @param[in] a     scalar to multiply by
     */
    vec2 operator*(float a) const { return {x * a, y * a}; }

    /**
     * Multiplication with another vec2
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] r     other vec2
     */
    vec2 operator*(const vec2 r) const { return {x * r.x, y * r.y}; }

    /**
     * Scalar division
     * As a safety measure, returns zero vector if scalar is not large enough (akin to division by zero)
     *
     * @param[in] r     scalar to divide by
     */
    vec2 operator/(const float r) const { return fabs(r) > bigger_epsilon ? vec2(x / r, y / r) : vec2(0, 0); }

    /**
     * Modulo operator on individual values
     * The values are guaranteed to be positive
     *
     * @param[in] r     scalar value to modulo with
     */
    vec2 operator%(const float r) const { // always return positive numbers
        float new_x = std::fmod(x ,r);
        new_x = new_x < 0.0f ? new_x + r : new_x;
        float new_y = std::fmod(y,r);
        new_y = new_y < 0.0f ? new_y + r : new_y;
        return {new_x, new_y};
    }

    /**
     * Vector addition
     *
     * @param[in] v     other vec2
     */
    vec2 operator+(const vec2& v) const { return {x + v.x, y + v.y}; }

    /**
     * Vector subtraction
     *
     * @param[in] v     other vec2
     */
    vec2 operator-(const vec2& v) const { return {x - v.x, y - v.y}; }

    /**
     * Returns a vector with values of the opposite signs
     */
    vec2 operator-() const { return {-x, -y}; }

    /**
     * Combined vector addition and assignment
     *
     * @param[in] v     other vec2
     */
    void operator+=(const vec2& v) { x += v.x, y += v.y;}

    /**
     * Combined scalar multiplication and assignment
     *
     * @param[in] a     scalar to multiply with
     */
    void operator*=(const float a) { x *= a, y *= a;}

    /**
     * Combined multiplication and assignment with another vec2
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] v     other vec2
     */
    void operator*=(const vec2& v) { x *= v.x, y *= v.y; }

    /**
     * Overloaded [] operator to access values in array-like way
     * Throws exception if index is not 0 or 1
     *
     * @param[in] idx   index to get value from
     */
    float operator[](const size_t idx) const {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            default:
                throw std::out_of_range("Index out of range [0-1]");
        }
    }

    /**
     * Returns length of the vector (euclidian norm)
     */
    float length() const { return sqrt(x * x + y * y); }

    /**
     * Returns the average of x and y
     */
    float average() const { return (x + y) / 2; }

    /**
     * Normalizes the vector
     */
    vec2 normalize() const { return (*this) / length(); }

    /**
     * Overloaded equals operator
     * Two vec2 instances are equal if their x and y values are equal
     *
     * @param[in] other     other vec2 to compare to
     */
    bool operator==(const vec2 & other) const { return (x == other.x) && (y == other.y); }
};

/**
 * Scalar multiplication in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
vec2 operator*(float a, const vec2& vector);

/**
 * Scalar division in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
vec2 operator/(float a, const vec2& vector);

/**
 * Overloaded out stream operator for printing
 *
 * @param[in,out] os    out stream
 * @param[in] vec       vec2 to print
 */
std::ostream& operator<<(std::ostream& os, const vec2& vec);

#endif //RAYTRACER_VEC2_HPP