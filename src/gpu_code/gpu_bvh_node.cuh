/**
 * @file gpu_bvh_node.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_CompactBVHNode class
 */

#pragma once

#include "gpu_bounding_box.cuh"
#include "../bvh_node.hpp"

/**
 * @class gpu_CompactBVHNode
 *
 * GPU compatible CompactBVHNode class
 */
class gpu_CompactBVHNode {
public:
    gpu_BoundingBox box;    /**< The bounding box of the BVH node, it contains a face if left and right children are SIZE_MAX */
    size_t left_idx;        /**< Index of the left child of the BVH node */
    size_t right_idx;       /**< Index of the right child of the BVH node */

    /**
     * Constructor from a CPU CompactBVHNode
     *
     * @param[in] n     CPU CompactBVHNode
     */
    __host__
    gpu_CompactBVHNode(const CompactBVHNode& n): box(n.box), left_idx(n.left_idx), right_idx(n.right_idx){}

};
