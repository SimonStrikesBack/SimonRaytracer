/**
 * @file gpu_vec3.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_vec3.cuh, see the header file for descriptions
 */
#include "gpu_vec3.cuh"
#include "gpu_vec2.cuh"
#include "../vec3.hpp"
__host__ __device__
gpu_vec3 operator*(const float a, const gpu_vec3& vector){
    return vector * a;
}
__host__ __device__
gpu_vec3 operator/(const float a, const gpu_vec3& vector) {
    return {a / vector.x, a / vector.y, a / vector.z};
}
__host__ __device__
gpu_vec3::gpu_vec3(const gpu_vec2 &v) {
    x = v.x;
    y = v.y;
    z = 0;
}
__host__ __device__
gpu_vec3::gpu_vec3(const gpu_vec2 &v, const float z): z(z) {
    x = v.x;
    y = v.y;
}

__host__
gpu_vec3::gpu_vec3(const vec3 &v): x(v.x), y(v.y), z(v.z) {}

__host__ __device__
void gpu_vec3::clamp(const float low, const float high) {
    x = gpu_clamp(x, low, high);
    y = gpu_clamp(y, low, high);
    z = gpu_clamp(z, low, high);
}