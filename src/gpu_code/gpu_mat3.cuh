/**
 * @file gpu_mat3.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_mat3 struct
 */

#pragma once
#include "gpu_vec3.cuh"
#include "gpu_geomath.cuh"
#include "../mat3.hpp"

/**
 * @struct gpu_mat3
 *
 * GPU compatible mat3 struct
 */
struct gpu_mat3 {
    gpu_vec3 x, y, z;       /**< Matrix's row vectors */

    /**
     * Zero value constructor
     */
    __host__ __device__
    gpu_mat3() {
        x = gpu_vec3(0.0f);
        y = gpu_vec3(0.0f);
        z = gpu_vec3(0.0f);
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
    __host__ __device__
    explicit gpu_mat3(const float a, const bool only_diagonal = true) {
        if (only_diagonal) {
            x = gpu_vec3(a, 0.0f, 0.0f);
            y = gpu_vec3(0.0f, a, 0.0f);
            z = gpu_vec3(0.0f, 0.0f, a);
        }
        else {
            x = gpu_vec3(a);
            y = gpu_vec3(a);
            z = gpu_vec3(a);
        }
    }

    /**
     * Constructor from a CPU mat3
     *
     * @param[in] m     CPU mat3
     */
    __host__
    gpu_mat3(const mat3& m): x(m.x), y(m.y), z(m.z){}

    /**
     * Constructs the matrix from the given row vectors
     *
     * @param[in] x     first row vector
     * @param[in] y     second row vector
     * @param[in] z     third row vector
     */
    __host__ __device__
    gpu_mat3(const gpu_vec3 x, const gpu_vec3 y, const gpu_vec3 z) : x(x), y(y), z(z) {}

    // Only basic operations so far
    /**
     * Multiplies the matrix by a scalar
     *
     * @param[in] a     scalar to multiply by
     */
    __host__ __device__
    gpu_mat3 operator*(const float a) const {return {x * a, y * a, z * a};}

    /**
     * Matrix addition
     *
     * @param[in] a     the other matrix
     */
    __host__ __device__
    gpu_mat3 operator+(const gpu_mat3 a) const{return {x + a.x, y + a.y, z + a.z};}

    /**
     * Matrix subtraction
     *
     * @param[in] a     the other matrix
     */
    __host__ __device__
    gpu_mat3 operator-(const gpu_mat3 a) const{return {x - a.x, y - a.y, z - a.z};}

    /**
     * Flips the sign of all matrix values
     */
    __host__ __device__
    gpu_mat3 operator-() const { return {-x, -y, -z}; }

    /**
     * Matrix vector multiplication from the left
     *
     * @param[in] a     vector to multiply by the matrix from the left
     */
    __host__ __device__
    gpu_vec3 operator*(const gpu_vec3 a) const{return {dot(x, a), dot(y, a), dot(z, a)};} // left matrix multiplication
};
