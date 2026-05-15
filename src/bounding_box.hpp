/**
 * @file bounding_box.hpp
 * @author Simon Tanev
 * @brief Header file containing the BoundingBox class
 */

#ifndef RAYTRACER_BOUNDING_BOX_HPP
#define RAYTRACER_BOUNDING_BOX_HPP
#pragma once

#include <memory>
#include "triangle.hpp"
#include "ray.hpp"
#include <algorithm>

/**
 * @class BoundingBox
 * Represents an Axis-Aligned Bounding Box around a face, which it holds a pointer to
 */
class BoundingBox {
public:
    const std::shared_ptr<triangle> face;   /**< The face contained in the bounding box */
    vec3 min;                               /**< One of the corners of the bounding box (minimal values) */
    vec3 max;                               /**< One of the corners of the bounding box (maximal values) */
    vec3 centroid;                          /**< Centroid of the bounding box */
    float surface_area;                     /**< Surface Area of the bounding box */

    /**
     * Constructor that creates a bounding box around a face
     * Other fields are calculated automatically
     *
     * @param[in]   face    Face to create the bounding box around
     */
    BoundingBox(const std::shared_ptr<triangle>& face): face(face) {
        const float min_x = std::min({face->a.position.x, face->b.position.x, face->c.position.x});
        const float min_y = std::min({face->a.position.y, face->b.position.y, face->c.position.y});
        const float min_z = std::min({face->a.position.z, face->b.position.z, face->c.position.z});

        const float max_x = std::max({face->a.position.x, face->b.position.x, face->c.position.x});
        const float max_y = std::max({face->a.position.y, face->b.position.y, face->c.position.y});
        const float max_z = std::max({face->a.position.z, face->b.position.z, face->c.position.z});

        min = vec3(min_x, min_y, min_z);
        max = vec3(max_x, max_y, max_z);
        centroid = Centroid();
        surface_area = SurfaceArea();
    }

    /**
     * Constructor that creates a bounding box by combining two other bounding boxes
     * The resulting Bounding Box does not hold a face (face is nullptr)
     * Other fields are calculated automatically
     *
     * @param[in]   a   One bounding box
     * @param[in]   b   The other bounding box
     */
    BoundingBox(const BoundingBox& a, const BoundingBox& b): face(nullptr){
        const float min_x = std::min(a.min.x, b.min.x);
        const float min_y = std::min(a.min.y, b.min.y);
        const float min_z = std::min(a.min.z, b.min.z);

        const float max_x = std::max(a.max.x, b.max.x);
        const float max_y = std::max(a.max.y, b.max.y);
        const float max_z = std::max(a.max.z, b.max.z);

        min = vec3(min_x, min_y, min_z);
        max = vec3(max_x, max_y, max_z);
        centroid = Centroid();
        surface_area = SurfaceArea();
    }

    /**
     * Constructor that creates a bounding box by combining a vector of bounding boxes
     * The resulting Bounding Box does not hold a face (face is nullptr)
     * Other fields are calculated automatically
     *
     * @param[in]   vector   Vector containing the bounding boxes to be combined
     */
    BoundingBox(const std::vector<BoundingBox>& vector): face(nullptr) {
        const auto min_x = std::ranges::min_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.min.x < b.min.x;
            });
        const auto min_y = std::ranges::min_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.min.y < b.min.y;
            });
        const auto min_z = std::ranges::min_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.min.z < b.min.z;
            });

        const auto max_x = std::ranges::max_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.max.x < b.max.x;
            });
        const auto max_y = std::ranges::max_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.max.y < b.max.y;
            });
        const auto max_z = std::ranges::max_element(vector,
            [](const BoundingBox &a, const BoundingBox &b) {
                return a.max.z < b.max.z;
            });

        min = vec3(min_x->min.x, min_y->min.y, min_z->min.z);
        max = vec3(max_x->max.x, max_y->max.y, max_z->max.z);
        centroid = Centroid();
        surface_area = SurfaceArea();
    }

    /**
     * Calculates and returns the centroid of the bounding box based on its current min and max values
     */
    vec3 Centroid() const;

    /**
     * Calculates and returns the surface area of the bounding box based on its current min and max values
     */
    float SurfaceArea() const;

    /**
     * Calculates and returns the t scaling value to an intersection of the ray with the bounding box
     * Returns infinity if there is no intersection, otherwise returns the closest intersection's t scale value
     *
     * @param[in]   ray     The ray to calculate the intersection with
     */
    float RayIntersection(const Ray& ray) const;
};


#endif //RAYTRACER_BOUNDING_BOX_HPP