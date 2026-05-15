/**
 * @file gpu_geomath.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_geomath.cuh, see the header file for descriptions
 */
#include "gpu_geomath.cuh"
#include "gpu_mat3.cuh"
#include <numbers>
#include "gpu_triangle.cuh"

__host__ __device__
float dot(const gpu_vec3& v1, const gpu_vec3& v2) { return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z); }

__host__ __device__
float dot(const gpu_vec2 &v1, const gpu_vec2 &v2) { return (v1.x * v2.x + v1.y + v2.y); }

__host__ __device__
gpu_vec3 cross(const gpu_vec3& v1, const gpu_vec3& v2) {
    return gpu_vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

__host__ __device__
gpu_vec3 reflect(const gpu_vec3& I, const gpu_vec3& N) {
    return I - 2.0f * dot(N, I) * N;
}

__host__ __device__
float distance(const gpu_vec2 &v1, const gpu_vec2 &v2) {
    return (v1 - v2).length();
}

__host__ __device__
float distance(const gpu_vec3 &v1, const gpu_vec3 &v2) {
    return (v1 - v2).length();
}

__host__ __device__
gpu_vec3 refract(const gpu_vec3 &I, const gpu_vec3 &N, const float refraction_first, const float ior_second) {
    // ior = 1/n_t
    // refraction = n
    const float factor = refraction_first * ior_second;
    return ((-factor) * I)+ (factor * (dot(I, N)) - sqrt(1.0f - pow(factor, 2) * (1.0f - pow(dot(I, N), 2.0f)))) * N;
}

__host__ __device__
float gpu_to_radians(const float degrees) {
    return degrees*(std::numbers::pi / 180.0f);
}

__host__ __device__
float gpu_to_degrees(const float radians) {
    return radians*(180.0f / std::numbers::pi);
}

__host__ __device__
// Code taken from the paper "Building an Orthonormal Basis, Revisited" by the Journal of Computer Graphics Techniques
void branchlessONB(const gpu_vec3& normal, gpu_vec3& tangent, gpu_vec3& bitangent) {
    const float sign = copysignf(1.0f, normal.z);
    const float a = -1.0f / (sign + normal.z);
    const float b = normal.x * normal.y * a;

    tangent = gpu_vec3(1.0f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
    bitangent = gpu_vec3(b, sign + normal.y * normal.y * a, -normal.y);
}

__host__ __device__
void normalOpenGLONB(const gpu_vec3 &normal, gpu_vec3 &tangent, gpu_vec3 &bitangent) {
    const gpu_vec3 up  = (fabs(normal.y) < 0.999f) ? gpu_vec3{0, 1, 0} : gpu_vec3{1, 0, 0};
    tangent = (up - normal * dot(normal, up)).normalize();
    bitangent = cross(normal, tangent);
}

__host__ __device__
void normalONB(const gpu_vec3 &normal, gpu_vec3 &tangent, gpu_vec3 &bitangent) {
    const gpu_vec3 up  = (fabs(normal.y) < 0.999f) ? gpu_vec3{0, 1, 0} : gpu_vec3{1, 0, 0};
    tangent = (up - normal * dot(normal, up)).normalize();
    bitangent = cross(tangent, normal);
}

__host__ __device__
void uvGradientsONB(const gpu_vec3 &normal, gpu_vec3 &tangent, gpu_vec3 &bitangent, const gpu_triangle& face) {
    const gpu_vec3 edge1 = face.b.position - face.a.position;
    const gpu_vec3 edge2 = face.c.position - face.a.position;

    const gpu_vec2 dUV1 = face.b.uv - face.a.uv;
    const gpu_vec2 dUV2 = face.c.uv - face.a.uv;

    const float det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;

    if (fabs(det) < 1e-8f) {
        // fallback (degenerate UVs)
        normalONB(normal, tangent, bitangent);
        return;
    }

    const float f = 1.0f / det;

    tangent = f * ( dUV2.y * edge1 - dUV1.y * edge2 );
    bitangent = f * (-dUV2.x * edge1 + dUV1.x * edge2 );

    tangent = (tangent - normal * dot(normal, tangent)).normalize();
    bitangent = cross(normal, tangent);

    if (dot(cross(normal, tangent), bitangent) < 0.0f) {
        tangent = -tangent;
    }
}

__host__ __device__
void localElevationAzimuth(const gpu_vec3 &world_ray_dir, const gpu_mat3 &world_to_local, float &elevation, float &azimuth) {
    const gpu_vec3 local_ray_dir = world_to_local * world_ray_dir;
    elevation = acosf(local_ray_dir.z);
    azimuth = atan2(local_ray_dir.y, local_ray_dir.x);
    if (azimuth < 0.0f) azimuth += 2.0f * std::numbers::pi;
}
