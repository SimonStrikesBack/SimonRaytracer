/**
 * @file object.hpp
 * @author Simon Tanev
 * @brief Header file containing the Object class
 */


#ifndef RAYTRACER_OBJECT_HPP
#define RAYTRACER_OBJECT_HPP
#pragma once
#include <vector>
#include <memory>
#include "triangle.hpp"

/**
 * @class Object
 *
 * Represents a scene object
 * It is just a wrapper around scene triangles
 */
class Object {
public:
    std::vector<std::shared_ptr<triangle>> faces;   /**< Object triangles */

    /**
     * Basic Constructor that just assigns all the input fields
     *
     * @param[in] triangles     triangles to assign
     */
    explicit Object(const std::vector<std::shared_ptr<triangle>>& triangles){
        faces = triangles;
    }
};


#endif //RAYTRACER_OBJECT_HPP
