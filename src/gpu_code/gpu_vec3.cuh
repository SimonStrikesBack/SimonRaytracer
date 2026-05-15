/**
 * @file gpu_vec3.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_vec3 struct
 */


#pragma once
#include "../constants.hpp"
#include <cmath>
#include <iostream>

#include "gpu_geomath.cuh"

struct gpu_vec2;
struct vec3;

/**
 * @class gpu_vec3
 *
 * GPU compatible vec3 struct
 */
struct gpu_vec3 {
    float x, y, z;      /**< X, Y and Z coordinates */

    /**
     * Constructor from a CPU vec3
     *
     * @param[in] v     CPU vec3
     */
    __host__
    gpu_vec3(const vec3 &v);

    /**
     * Constructs a gpu_vec3 from a gpu_vec2 by appending a zero element
     *
     * @param[in] v     gpu_vec2 to construct from
     */
    __host__ __device__
    gpu_vec3(const gpu_vec2& v);

    /**
     * Constructs a gpu_vec3 from a gpu_vec2 by appending the passed scalar value
     *
     * @param[in] v     gpu_vec2 to construct from
     * @param[in] z     scalar value to append to z
     */
    __host__ __device__
    gpu_vec3(const gpu_vec2& v, float z);

    /**
     * Basic zero initializer constructor
     */
    __host__ __device__
    gpu_vec3() : x(0), y(0), z(0) {}

    /**
     * Constructs a gpu_vec3 from a single scalar value
     *
     * @param[in] v     value to populate x and y
     */
    __host__ __device__
    gpu_vec3(const float v) : x(v), y(v), z(v) {}

    /**
     * Constructs a gpu_vec3 from a specified x, y and z values
     *
     * @param[in] x     x value
     * @param[in] y     y value
     * @param[in] z     z value
     */
    __host__ __device__
    gpu_vec3(const float x, const float y, const float z = 0) : x(x), y(y), z(z) {}

    /**
     * Constructs a gpu_vec3 from a static 3 value array
     *
     * @param[in] vector    static array of size 3 to get values from
     */
    __host__ __device__
    gpu_vec3(float vector[3]): x(vector[0]), y(vector[1]), z(vector[2]){}

    /**
     * Scalar multiplication
     *
     * @param[in] a     scalar to multiply by
     */
    __host__ __device__
    gpu_vec3 operator*(const float a) const { return {x * a, y * a, z * a}; }

    /**
     * Multiplication with another gpu_vec3
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] r     other gpu_vec3
     */
    __host__ __device__
    gpu_vec3 operator*(const gpu_vec3 r) const { return {x * r.x, y * r.y, z * r.z}; }

    /**
     * Scalar division
     * As a safety measure, returns zero vector if scalar is not large enough (akin to division by zero)
     *
     * @param[in] r     scalar to divide by
     */
    __host__ __device__
    gpu_vec3 operator/(const float r) const { return fabs(r) > bigger_epsilon ? gpu_vec3(x / r, y / r, z / r) : gpu_vec3(0, 0, 0); }

    /**
     * Modulo operator on individual values
     * The values are guaranteed to be positive
     *
     * @param[in] r     scalar value to modulo with
     */
    __host__ __device__
    gpu_vec3 operator%(const float r) const { // always return positive numbers
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
     * @param[in] v     other gpu_vec3
     */
    __host__ __device__
    gpu_vec3 operator+(const gpu_vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }

    /**
     * Vector subtraction
     *
     * @param[in] v     other gpu_vec3
     */
    __host__ __device__
    gpu_vec3 operator-(const gpu_vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }

    /**
     * Returns a vector with values of the opposite signs
     */
    __host__ __device__
    gpu_vec3 operator-() const { return {-x, -y, -z}; }

    /**
     * Combined vector addition and assignment
     *
     * @param[in] v     other gpu_vec3
     */
    __host__ __device__
    void operator+=(const gpu_vec3& v) { x += v.x, y += v.y, z += v.z; }

    /**
     * Combined scalar multiplication and assignment
     *
     * @param[in] a     scalar to multiply with
     */
    __host__ __device__
    void operator*=(const float a) { x *= a, y *= a, z *= a; }

    /**
     * Combined multiplication and assignment with another gpu_vec3
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] v     other gpu_vec3
     */
    __host__ __device__
    void operator*=(const gpu_vec3& v) { x *= v.x, y *= v.y, z *= v.z; }

    /**
     * Overloaded [] operator to access values in array-like way
     * Returns INFINITY if index is not 0, 1 or 2
     *
     * @param[in] idx   index to get value from
     */
    __host__ __device__
    float operator[](const size_t idx) const {
        switch (idx) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return INFINITY;
        }
    }

    /**
     * Returns length of the vector (euclidian norm)
     */
    __host__ __device__
    float length() const { return sqrt(x * x + y * y + z * z); }

    /**
     * Returns the average of x, y and z
     */
    __host__ __device__
    float average() const { return (x + y + z) / 3; }

    /**
     * Normalizes the vector
     */
    __host__ __device__
    gpu_vec3 normalize() const { return (*this) / length(); }

    /**
     * Clamps all values between low and high
     *
     * @param[in] low       low clamp value
     * @param[in] high      high clamp value
     */
    __host__ __device__
    void clamp(const float low, const float high);

    /**
     * Overloaded equals operator
     * Two gpu_vec3 instances are equal if their x, y and z values are equal
     *
     * @param[in] other     other gpu_vec3 to compare to
     */
    __host__ __device__
    bool operator==(const gpu_vec3 & other) const { return (x == other.x) && (y == other.y) && (z == other.z); }
};

/**
 * Scalar multiplication in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
__host__ __device__
gpu_vec3 operator*(float a, const gpu_vec3& vector);

/**
 * Scalar division in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
__host__ __device__
gpu_vec3 operator/(float a, const gpu_vec3& vector);

