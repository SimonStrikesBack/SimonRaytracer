/**
 * @file gpu_object.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Object class
 */

#pragma once
#include <cuda_runtime.h>

#include "gpu_triangle.cuh"
#include "gpu_material.cuh"
#include "../object.hpp"

/**
 * @class gpu_Object
 *
 * GPU compatible Object class
 */
class gpu_Object {
public:
    gpu_triangle* faces;        /**< Object triangles */

    /**
     * Constructor from a CPU Object
     *
     * @param[in] o     CPU Object
     */
    __host__
    gpu_Object(const Object& o) {
        cudaMallocManaged(&faces, o.faces.size() * sizeof(gpu_triangle));
        for (size_t i = 0; i < o.faces.size(); i++) {
            faces[i] = gpu_triangle(*o.faces[i]);
        }
    }

    /**
     * Destructor of the class
     * It calls free functions on the CUDA memory stored
     */
    __host__
    ~gpu_Object() {
        cudaFree(faces);
    }
};


