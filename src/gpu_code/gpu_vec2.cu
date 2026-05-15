/**
 * @file gpu_vec2.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_vec2.cuh, see the header file for descriptions
 */

#include "gpu_vec2.cuh"
#include "gpu_vec3.cuh"
#include "../vec2.hpp"

__host__ __device__
gpu_vec2 operator*(const float a, const gpu_vec2& vector) {
    return vector * a;
}
__host__ __device__
gpu_vec2 operator/(const float a, const gpu_vec2& vector) {
    return {a / vector.x, a / vector.y};
}
__host__ __device__
gpu_vec2::gpu_vec2(const gpu_vec3 &v) {
    x = v.x;
    y = v.y;
}

__host__
gpu_vec2::gpu_vec2(const vec2 &v): x(v.x), y(v.y) {}
