/**
 * @file bounding_box.cpp
 * @author Simon Tanev
 * @brief Implementation file for bounding_box.hpp, see the header file for descriptions
 */

#include "bounding_box.hpp"

float BoundingBox::RayIntersection(const Ray &ray) const {
    float t1 = (min.x - ray.origin.x) * ray.inv_dir.x;
    float t2 = (max.x - ray.origin.x) * ray.inv_dir.x;
    float tmin = std::min(t1, t2);
    float tmax = std::max(t1, t2);

    t1 = (min.y - ray.origin.y) * ray.inv_dir.y;
    t2 = (max.y - ray.origin.y) * ray.inv_dir.y;
    tmin = std::max(tmin, std::min(t1, t2));
    tmax = std::min(tmax, std::max(t1, t2));

    t1 = (min.z - ray.origin.z) * ray.inv_dir.z;
    t2 = (max.z - ray.origin.z) * ray.inv_dir.z;
    tmin = std::max(tmin, std::min(t1, t2));
    tmax = std::min(tmax, std::max(t1, t2));

    if (tmax < std::max(tmin, 0.0f))
        return INFINITY;

    if (tmin < 0.0f)
        return tmax;

    return tmin;
}

vec3 BoundingBox::Centroid() const {
    const float x = (min.x + max.x) * 0.5f;
    const float y = (min.y + max.y) * 0.5f;
    const float z = (min.z + max.z) * 0.5f;
    return {x, y, z};
}

float BoundingBox::SurfaceArea() const {
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float depth = max.z - min.z;
    return 2.0f * (width*height + width*depth + height*depth);
}
