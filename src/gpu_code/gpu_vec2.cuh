/**
 * @file gpu_vec2.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_vec2 struct
 */

#pragma once
#include <iostream>
#include "../constants.hpp"

struct gpu_vec3;
struct vec2;

/**
 * @class gpu_vec2
 *
 * GPU compatible vec2 struct
 */
struct gpu_vec2 {
    float x, y;     /**< X and Y coordinates */

    /**
     * Constructor from a CPU vec2
     *
     * @param[in] v     CPU vec2
     */
    __host__
    gpu_vec2(const vec2& v);

    /**
     * Constructs a gpu_vec2 from a gpu_vec3 by cutting off its z component
     *
     * @param[in] v     gpu_vec3 to construct from
     */
    __host__ __device__
    gpu_vec2(const gpu_vec3& v);

    /**
     * Basic zero initializer constructor
     */
    __host__ __device__
    gpu_vec2() : x(0), y(0) {}

    /**
     * Constructs a gpu_vec2 from a single scalar value
     *
     * @param[in] v     value to populate x and y
     */
    __host__ __device__
    gpu_vec2(const float v) : x(v), y(v) {}

    /**
     * Constructs a gpu_vec2 from a specified x and y values
     *
     * @param[in] x     x value
     * @param[in] y     y value
     */
    __host__ __device__
    gpu_vec2(const float x, const float y) : x(x), y(y) {}

    /**
     * Constructs a gpu_vec2 from a static 3 value array, uses the first two values as x and y
     *
     * @param[in] vector    static array of size 3 to get values from
     */
    __host__ __device__
    gpu_vec2(float vector[3]): x(vector[0]), y(vector[1]) {}

    /**
     * Scalar multiplication
     *
     * @param[in] a     scalar to multiply by
     */
    __host__ __device__
    gpu_vec2 operator*(float a) const { return {x * a, y * a}; }

    /**
     * Multiplication with another gpu_vec2
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] r     other gpu_vec2
     */
    __host__ __device__
    gpu_vec2 operator*(const gpu_vec2 r) const { return {x * r.x, y * r.y}; }

    /**
     * Scalar division
     * As a safety measure, returns zero vector if scalar is not large enough (akin to division by zero)
     *
     * @param[in] r     scalar to divide by
     */
    __host__ __device__
    gpu_vec2 operator/(const float r) const { return fabs(r) > bigger_epsilon ? gpu_vec2(x / r, y / r) : gpu_vec2(0, 0); }

    /**
     * Modulo operator on individual values
     * The values are guaranteed to be positive
     *
     * @param[in] r     scalar value to modulo with
     */
    __host__ __device__
    gpu_vec2 operator%(const float r) const { // always return positive numbers
        float new_x = std::fmod(x ,r);
        new_x = new_x < 0.0f ? new_x + r : new_x;
        float new_y = std::fmod(y,r);
        new_y = new_y < 0.0f ? new_y + r : new_y;
        return {new_x, new_y};
    }

    /**
     * Vector addition
     *
     * @param[in] v     other gpu_vec2
     */
    __host__ __device__
    gpu_vec2 operator+(const gpu_vec2& v) const { return {x + v.x, y + v.y}; }

    /**
     * Vector subtraction
     *
     * @param[in] v     other gpu_vec2
     */
    __host__ __device__
    gpu_vec2 operator-(const gpu_vec2& v) const { return {x - v.x, y - v.y}; }

    /**
     * Returns a vector with values of the opposite signs
     */
    __host__ __device__
    gpu_vec2 operator-() const { return {-x, -y}; }

    /**
     * Combined vector addition and assignment
     *
     * @param[in] v     other gpu_vec2
     */
    __host__ __device__
    void operator+=(const gpu_vec2& v) { x += v.x, y += v.y;}

    /**
     * Combined scalar multiplication and assignment
     *
     * @param[in] a     scalar to multiply with
     */
    __host__ __device__
    void operator*=(const float a) { x *= a, y *= a;}

    /**
     * Combined multiplication and assignment with another gpu_vec2
     * Not a dot product, but a multiplication of the individual values
     *
     * @param[in] v     other gpu_vec2
     */
    __host__ __device__
    void operator*=(const gpu_vec2& v) { x *= v.x, y *= v.y; }

    /**
     * Overloaded [] operator to access values in array-like way
     * Returns INFINITY if index is not 0 or 1
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
            default:
                return INFINITY;
        }
    }

    /**
     * Returns length of the vector (euclidian norm)
     */
    __host__ __device__
    float length() const { return sqrt(x * x + y * y); }

    /**
     * Returns the average of x and y
     */
    __host__ __device__
    float average() const { return (x + y) / 2; }

    /**
     * Normalizes the vector
     */
    __host__ __device__
    gpu_vec2 normalize() const { return (*this) / length(); }

    /**
     * Overloaded equals operator
     * Two gpu_vec2 instances are equal if their x and y values are equal
     *
     * @param[in] other     other gpu_vec2 to compare to
     */
    __host__ __device__
    bool operator==(const gpu_vec2 & other) const { return (x == other.x) && (y == other.y); }
};

/**
 * Scalar multiplication in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
__host__ __device__
gpu_vec2 operator*(float a, const gpu_vec2& vector);

/**
 * Scalar division in the other order of arguments
 *
 * @param[in] a         scalar value
 * @param[in] vector    vector
 */
__host__ __device__
gpu_vec2 operator/(float a, const gpu_vec2& vector);
