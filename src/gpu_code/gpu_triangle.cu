/**
 * @file gpu_triangle.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_triangle.cuh, see the header file for descriptions
 */

#include "gpu_triangle.cuh"
#include "gpu_geomath.cuh"
#include <cmath>

float gpu_triangle::intersect(const gpu_Ray &ray, bool cullback, float &b1, float &b2, const float epsilon) const{
    const gpu_vec3 e1(b.position - a.position);
    const gpu_vec3 e2(c.position - a.position);
    gpu_vec3 pvec = cross(ray.dir, e2);
    const float det = dot(e1, pvec);

    if (cullback)
    {
        if (det < epsilon) // ray is parallel to triangle
            return INFINITY;
    }
    else
    {
        if (fabs(det) < epsilon) // ray is parallel to triangle
            return INFINITY;
    }

    float invDet = 1.0f / det;

    // Compute first barycentric coordinate
    gpu_vec3 tvec = ray.origin - a.position;
    b1 = dot(tvec, pvec) * invDet;

    if (b1 < 0.0f || b1 > 1.0f)
        return INFINITY;

    // Compute second barycentric coordinate
    gpu_vec3 qvec = cross(tvec, e1);
    b2 = dot(ray.dir, qvec) * invDet;

    if (b2 < 0.0f || b1 + b2 > 1.0f)
        return INFINITY;

    // Compute t to intersection point
    const float t = dot(e2, qvec) * invDet;
    return t;
}

float gpu_triangle::area() const {
    return 0.5f * cross((b.position - a.position), (c.position - a.position)).length();
}
