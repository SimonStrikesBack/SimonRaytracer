/**
 * @file vec3.hpp
 * @author Simon Tanev
 * @brief File containing vec3 struct
 */

#ifndef RAYTRACER_VEC3_HPP
#define RAYTRACER_VEC3_HPP
#pragma once
#include "constants.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>

struct vec2;

/**
 * @struct vec3
 *
 * Represents a 3-dimensional vector
 */
struct vec3 {
    float x, y, z;  /**< X, Y and Z coordinates */

    /**
     * Constructs a vec3 from a vec2 by appending a zero element
     *
     * @param[in] v     vec2 to construct from
     */
    vec3(const vec2& v);

    /**
     * Constructs a vec3 from a vec2 by appending the passed scalar value
     *
     * @param[in] v     vec2 to construct from
     * @param[in] z     scalar value to append to z
     */
    vec3(const vec2& v, float z);

    /**
     * Basic zero initializer constructor
     */
    vec3() : x(0), y(0), z(0) {}

    /**
     * Constructs a vec3 from a single scalar value
     *
     * @param[in] v     value to populate x and y
     */
    vec3(const float v) : x(v), y(v), z(v) {}

    /**
     * Constructs a vec3 from a specified x, y and z values
     *
     * @param[in] x     x value
     * @param[in] y     y value
     * @param[in] z     z value
     */
    vec3(const float x, const float y, const float z = 0) : x(x), y(y), z(z) {}

    /**
     * Constructs a vec3 from a static 3 value array
     *
     * @param[in] vector    static array of size 3 to get values from
     */
    vec3(float vector[3]): x(vector[0]), y(vector[1]), z(vector[2]){}

    /**
     * Constructs a vec3 from a c++ vector of floats
     * It also asserts that there are enough values to construct a vec3
     *
     * @param[in] v     vector to get values from
     */
    vec3(const std::vector<float> &v){
        assert(v.size() > 2);
        x = v[0];
        y = v[1];
        z = v[2];
    }

    /**
     * Scalar multiplication
     *
     * @param[in] a     scalar to multiply by
     */
    vec3 operator*(const float a) const { return {x * a, y * a, z * a}; }

    /**
     * Multiplication with another vec3
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] r     other vec3
     */
    vec3 operator*(const vec3 r) const { return {x * r.x, y * r.y, z * r.z}; }

    /**
     * Scalar division
     * As a safety measure, returns zero vector if scalar is not large enough (akin to division by zero)
     *
     * @param[in] r     scalar to divide by
     */
    vec3 operator/(const float r) const { return fabs(r) > bigger_epsilon ? vec3(x / r, y / r, z / r) : vec3(0, 0, 0); }

    /**
     * Modulo operator on individual values
     * The values are guaranteed to be positive
     *
     * @param[in] r     scalar value to modulo with
     */
    vec3 operator%(const float r) const { // always return positive numbers
        float new_x = std::fmod(x ,r);
        new_x = new_x < 0.0f ? new_x + r : new_x;
        float new_y = std::fmod(y,r);
        new_y = new_y < 0.0f ? new_y + r : new_y;
        float new_z = std::fmod(z,r);
        new_z = new_z < 0.0f ? new_z + r : new_z;
        return {new_x, new_y, new_z};
    }

    /**
     * Vector addition
     *
     * @param[in] v     other vec3
     */
    vec3 operator+(const vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }

    /**
     * Vector subtraction
     *
     * @param[in] v     other vec3
     */
    vec3 operator-(const vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }

    /**
     * Returns a vector with values of the opposite signs
     */
    vec3 operator-() const { return {-x, -y, -z}; }

    /**
     * Combined vector addition and assignment
     *
     * @param[in] v     other vec3
     */
    void operator+=(const vec3& v) { x += v.x, y += v.y, z += v.z; }

    /**
     * Combined scalar multiplication and assignment
     *
     * @param[in] a     scalar to multiply with
     */
    void operator*=(const float a) { x *= a, y *= a, z *= a; }

    /**
     * Combined multiplication and assignment with another vec3
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] v     other vec3
     */
    void operator*=(const vec3& v) { x *= v.x, y *= v.y, z *= v.z; }

    /**
     * Overloaded [] operator to access values in array-like way
     * Throws exception if index is not 0, 1 or 2
     *
     * @param[in] idx   index to get value from
     */
    float operator[](const size_t idx) const {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                throw std::out_of_range("Index out of range [0-2]");
        }
    }

    /**
     * Returns length of the vector (euclidian norm)
     */
    float length() const { return sqrt(x * x + y * y + z * z); }

    /**
     * Returns the average of x, y and z
     */
    float average() const { return (x + y + z) / 3; }

    /**
     * Normalizes the vector
     */
    vec3 normalize() const { return (*this) / length(); }

    /**
     * Clamps all values between low and high
     *
     * @param[in] low       low clamp value
     * @param[in] high      high clamp value
     */
    void vec_clamp(const float low, const float high);

    /**
     * Overloaded equals operator
     * Two vec3 instances are equal if their x, y and z values are equal
     *
     * @param[in] other     other vec3 to compare to
     */
    bool operator==(const vec3 & other) const { return (x == other.x) && (y == other.y) && (z == other.z); }
};

/**
 * Scalar multiplication in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
vec3 operator*(float a, const vec3& vector);

/**
 * Scalar division in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
vec3 operator/(float a, const vec3& vector);

/**
 * Overloaded out stream operator for printing
 *
 * @param[in,out] os    out stream
 * @param[in] vec       vec3 to print
 */
std::ostream& operator<<(std::ostream& os, const vec3& vec);

#endif //RAYTRACER_VEC3_HPP
