/**
 * @file gpu_geomath.cuh
 * @author Simon Tanev
 * @brief Header file containing gpu compatible geometric and mathematical functions for vectors and matrices
 */

#pragma once
#include "gpu_vec3.cuh"
#include "gpu_vec2.cuh"

struct gpu_mat3;
class gpu_triangle;


/**
 * Templated GPU compatible max function
 * Returns the larger value
 *
 * @param a     one value
 * @param b     other value
 */
template<typename T>
__host__ __device__
T gpu_max(T a, T b) {
    return (a > b) ? a : b;
}

/**
 * Templated GPU compatible min function
 * Returns the smaller value
 *
 * @param a     one value
 * @param b     other value
 */
template<typename T>
__host__ __device__
T gpu_min(T a, T b) {
    return (a < b) ? a : b;
}

/**
 * GPU compatible dot product of two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
__host__ __device__
float dot(const gpu_vec3& v1, const gpu_vec3& v2);

/**
 * GPU compatible dot product of two 2-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
__host__ __device__
float dot(const gpu_vec2& v1, const gpu_vec2& v2);

/**
 * GPU compatible cross product of two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
__host__ __device__
gpu_vec3 cross(const gpu_vec3& v1, const gpu_vec3& v2);

/**
 * Reflects the vector I around a normal vector N (GPU compatible)
 *
 * @param[in] I    vector to be reflected
 * @param[in] N    normal vector
 */
__host__ __device__
gpu_vec3 reflect(const gpu_vec3& I, const gpu_vec3& N);

/**
 * Refracts the vector I around a normal vector N, based on the ior of the materials (GPU compatible)
 *
 * @param[in] I                     vector to be refracted
 * @param[in] N                     normal vector
 * @param[in] refraction_first      refraction index of the first material
 * @param[in] ior_second            ior of the second material
 *
 * refraction index = 1 / ior
 * ior = 1 / refraction index
 */
__host__ __device__
gpu_vec3 refract(const gpu_vec3& I, const gpu_vec3& N, const float refraction_first, const float ior_second);

/**
 * GPU compatible euclidian distance between two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
__host__ __device__
float distance(const gpu_vec3& v1, const gpu_vec3& v2);

/**
 * GPU compatible euclidian distance between two 2-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
__host__ __device__
float distance(const gpu_vec2& v1, const gpu_vec2& v2);

/**
 * Constructs an Orthonormal base from the input normal vector (GPU compatible)
 * Based on the paper "Building an Orthonormal Basis, Revisited" by the Journal of Computer Graphics Techniques
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
__host__ __device__
void branchlessONB(const gpu_vec3& normal, gpu_vec3& tangent, gpu_vec3& bitangent);

/**
 * Constructs an Orthonormal base from the input normal vector (with the OpenGL winding in mind, GPU compatible)
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
__host__ __device__
void normalOpenGLONB(const gpu_vec3& normal, gpu_vec3& tangent, gpu_vec3& bitangent);

/**
 * Constructs an Orthonormal base from the input normal vector (GPU compatible)
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
__host__ __device__
void normalONB(const gpu_vec3 &normal, gpu_vec3 &tangent, gpu_vec3 &bitangent);

/**
 * Constructs an Orthonormal base for normal mapping from the input normal vector based on the UV Gradient method (GPU compatible)
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 * @param[in]       face        face to construct the UV gradient ONB for
 */
__host__ __device__
void uvGradientsONB(const gpu_vec3 &normal, gpu_vec3 &tangent, gpu_vec3 &bitangent, const gpu_triangle& face);

/**
 * Calculates the local coordinate elevation and azimuth for a direction vector in world coordinates (GPU compatible)
 *
 * @param[in]   world_ray_dir   direction vector in world coordinates to calculate elevation and azimuth for
 * @param[in]   world_to_local  world to local coordinate conversion matrix
 * @param[out]  elevation       calculated local elevation
 * @param[out]  azimuth         calculated local azimuth
 */
__host__ __device__
void localElevationAzimuth(const gpu_vec3& world_ray_dir, const gpu_mat3& world_to_local, float& elevation, float& azimuth );

/**
 * Templated clamp function, clamps the value between low and high (GPU compatible)
 *
 * @param[in]   val     value to clamp
 * @param[in]   low     lower clamp value
 * @param[in]   high    higher clamp value
 */
template <typename T>
__host__ __device__
inline T gpu_clamp(T val, T low, T high) {
    return gpu_max(gpu_min(val, high), low);
}

