/**
 * @file mat3.hpp
 * @author Simon Tanev
 * @brief Header file containing the mat3 struct
 */

#ifndef RAYTRACER_MAT3_HPP
#define RAYTRACER_MAT3_HPP
#pragma once
#include "vec3.hpp"
#include "geomath.hpp"

/**
 * @struct mat3
 *
 * Represents a 3x3 Matrix
 * It stores the matrix as 3 3-dimensional row vectors
 */
struct mat3 {
    vec3 x, y, z;   /**< Matrix's row vectors */

    /**
     * Zero value constructor
     */
    mat3() {
        x = vec3(0.0f);
        y = vec3(0.0f);
        z = vec3(0.0f);
    }
    // only diagonal is on by default to be the same as in glm

    /**
     * Constructs the matrix from a given single value
     *
     * If only diagonal is true (default), the input value is only on the diagonal, rest of the matrix is zero
     * Otherwise the value is used in the whole matrix
     *
     * @param[in] a                 value of the matrix elements
     * @param[in] only_diagonal     whether to use the value only on the diagonal
     */
    explicit mat3(const float a, const bool only_diagonal = true) {
        if (only_diagonal) {
            x = vec3(a, 0.0f, 0.0f);
            y = vec3(0.0f, a, 0.0f);
            z = vec3(0.0f, 0.0f, a);
        }
        else {
            x = vec3(a);
            y = vec3(a);
            z = vec3(a);
        }
    }

    /**
     * Constructs the matrix from the given row vectors
     *
     * @param[in] x     first row vector
     * @param[in] y     second row vector
     * @param[in] z     third row vector
     */
    mat3(const vec3 x, const vec3 y, const vec3 z) : x(x), y(y), z(z) {}

    // Only basic operations so far
    /**
     * Multiplies the matrix by a scalar
     *
     * @param[in] a     scalar to multiply by
     */
    mat3 operator*(const float a) const {return {x * a, y * a, z * a};}

    /**
     * Matrix addition
     *
     * @param[in] a     the other matrix
     */
    mat3 operator+(const mat3 a) const{return {x + a.x, y + a.y, z + a.z};}

    /**
     * Matrix subtraction
     *
     * @param[in] a     the other matrix
     */
    mat3 operator-(const mat3 a) const{return {x - a.x, y - a.y, z - a.z};}

    /**
     * Flips the sign of all matrix values
     */
    mat3 operator-() const { return {-x, -y, -z}; }

    /**
     * Matrix vector multiplication from the left
     *
     * @param[in] a     vector to multiply by the matrix from the left
     */
    vec3 operator*(const vec3 a) const{return {dot(x, a), dot(y, a), dot(z, a)};} // left matrix multiplication
};


#endif //RAYTRACER_MAT3_HPP