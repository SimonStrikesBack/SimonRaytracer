/**
 * @file geomath.hpp
 * @author Simon Tanev
 * @brief Header file containing geometric and mathematical functions for vectors and matrices
 */


#ifndef RAYTRACER_GEOMATH_HPP
#define RAYTRACER_GEOMATH_HPP
#pragma once
#include "vec3.hpp"
#include "vec2.hpp"

// Forward declarations
struct mat3;
class triangle;

/**
 * Dot product of two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
float dot(const vec3& v1, const vec3& v2);

/**
 * Dot product of two 2-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
float dot(const vec2& v1, const vec2& v2);

/**
 * Cross product of two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
vec3 cross(const vec3& v1, const vec3& v2);

/**
 * Reflects the vector I around a normal vector N
 *
 * @param[in] I    vector to be reflected
 * @param[in] N    normal vector
 */
vec3 reflect(const vec3& I, const vec3& N);

/**
 * Refracts the vector I around a normal vector N, based on the ior of the materials
 *
 * @param[in] I                     vector to be refracted
 * @param[in] N                     normal vector
 * @param[in] refraction_first      refraction index of the first material
 * @param[in] ior_second            ior of the second material
 *
 * refraction index = 1 / ior
 * ior = 1 / refraction index
 */
vec3 refract(const vec3& I, const vec3& N, const float refraction_first, const float ior_second);

/**
 * Euclidian distance between two 3-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
float distance(const vec3& v1, const vec3& v2);

/**
 * Euclidian distance between two 2-dimensional vectors
 *
 * @param[in] v1    first vector
 * @param[in] v2    second vector
 */
float distance(const vec2& v1, const vec2& v2);

/**
 * Constructs an Orthonormal base from the input normal vector
 * Based on the paper "Building an Orthonormal Basis, Revisited" by the Journal of Computer Graphics Techniques
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
void branchlessONB(const vec3& normal, vec3& tangent, vec3& bitangent);

/**
 * Constructs an Orthonormal base from the input normal vector (with the OpenGL winding in mind)
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
void normalOpenGLONB(const vec3& normal, vec3& tangent, vec3& bitangent);

/**
 * Constructs an Orthonormal base from the input normal vector
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 */
void normalONB(const vec3 &normal, vec3 &tangent, vec3 &bitangent);

/**
 * Constructs an Orthonormal base for normal mapping from the input normal vector based on the UV Gradient method
 *
 * @param[in]       normal      normal vector to build the ONB from
 * @param[out]      tangent     tangent vector of the constructed ONB
 * @param[out]      bitangent   bitangent vector of the constructed ONB
 * @param[in]       face        face to construct the UV gradient ONB for
 */
void uvGradientsONB(const vec3 &normal, vec3 &tangent, vec3 &bitangent, const triangle& face);

/**
 * Calculates the local coordinate elevation and azimuth for a direction vector in world coordinates
 *
 * @param[in]   world_ray_dir   direction vector in world coordinates to calculate elevation and azimuth for
 * @param[in]   world_to_local  world to local coordinate conversion matrix
 * @param[out]  elevation       calculated local elevation
 * @param[out]  azimuth         calculated local azimuth
 */
void localElevationAzimuth(const vec3& world_ray_dir, const mat3& world_to_local, float& elevation, float& azimuth );

/**
 * Templated clamp function, clamps the value between low and high
 *
 * @param[in]   val     value to clamp
 * @param[in]   low     lower clamp value
 * @param[in]   high    higher clamp value
 */
template <typename T>
inline T clamp(T val, T low, T high) {
    return std::max(std::min(val, high), low);
}

#endif //RAYTRACER_GEOMATH_HPP
