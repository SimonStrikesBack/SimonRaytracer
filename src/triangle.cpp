/**
 * @file triangle.cpp
 * @author Simon Tanev
 * @brief Implementation file for triangle.hpp, see the header file for descriptions
 */

#include "triangle.hpp"
#include "geomath.hpp"
#include <cmath>

float triangle::intersect(const Ray &ray, bool cullback, float &b1, float &b2, const float epsilon) const{
    const vec3 e1(b.position - a.position);
    const vec3 e2(c.position - a.position);
    vec3 pvec = cross(ray.dir, e2);
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
    vec3 tvec = ray.origin - a.position;
    b1 = dot(tvec, pvec) * invDet;

    if (b1 < 0.0f || b1 > 1.0f)
        return INFINITY;

    // Compute second barycentric coordinate
    vec3 qvec = cross(tvec, e1);
    b2 = dot(ray.dir, qvec) * invDet;

    if (b2 < 0.0f || b1 + b2 > 1.0f)
        return INFINITY;

    // Compute t to intersection point
    const float t = dot(e2, qvec) * invDet;
    return t;
}

float triangle::area() const {
    return 0.5f * cross((b.position - a.position), (c.position - a.position)).length();
}
