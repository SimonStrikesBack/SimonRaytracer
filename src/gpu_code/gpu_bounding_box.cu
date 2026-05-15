/**
 * @file gpu_bounding_box.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_bounding_box.cuh, see the header file for descriptions
 */

#include "gpu_bounding_box.cuh"

__host__ __device__
float gpu_BoundingBox::RayIntersection(const gpu_Ray &ray) const {
    float t1 = (min.x - ray.origin.x) * ray.inv_dir.x;
    float t2 = (max.x - ray.origin.x) * ray.inv_dir.x;
    float tmin = gpu_min(t1, t2);
    float tmax = gpu_max(t1, t2);

    t1 = (min.y - ray.origin.y) * ray.inv_dir.y;
    t2 = (max.y - ray.origin.y) * ray.inv_dir.y;
    tmin = gpu_max(tmin, gpu_min(t1, t2));
    tmax = gpu_min(tmax, gpu_max(t1, t2));

    t1 = (min.z - ray.origin.z) * ray.inv_dir.z;
    t2 = (max.z - ray.origin.z) * ray.inv_dir.z;
    tmin = gpu_max(tmin, gpu_min(t1, t2));
    tmax = gpu_min(tmax, gpu_max(t1, t2));

    if (tmax < gpu_max(tmin, 0.0f))
        return INFINITY;

    if (tmin < 0.0f)
        return tmax;

    return tmin;
}

__host__ __device__
gpu_vec3 gpu_BoundingBox::Centroid() const {
    const float x = (min.x + max.x) * 0.5f;
    const float y = (min.y + max.y) * 0.5f;
    const float z = (min.z + max.z) * 0.5f;
    return {x, y, z};
}

__host__ __device__
float gpu_BoundingBox::SurfaceArea() const {
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float depth = max.z - min.z;
    return 2.0f * (width*height + width*depth + height*depth);
}
