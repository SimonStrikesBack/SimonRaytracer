/**
 * @file bvh_node.hpp
 * @author Simon Tanev
 * @brief Header file containing the BVHNode, CompactBVHNode classes and SAH helper structs
 */

#ifndef RAYTRACER_BVH_NODE_HPP
#define RAYTRACER_BVH_NODE_HPP
#pragma once

#include "bounding_box.hpp"
#include <omp.h>

struct SAHResult;

/**
 * @class BVHNode
 * Represents a node in the BVH tree
 */
class BVHNode {
public:
    const BoundingBox box;              /**< The bounding box of the BVH node, it contains a face if left and right children are nullptr */
    std::shared_ptr<BVHNode> left;      /**< Left child of the BVH node */
    std::shared_ptr<BVHNode> right;     /**< Right child of the BVH node */

    //static int parallelism_threshold;

    /**
     * Basic Constructor that just assigns all the input fields
     *
     * @param[in]   box     The bounding box of the node
     * @param[in]   left    The left child of the node
     * @param[in]   right   The right child of the node
     */
    BVHNode(const BoundingBox& box, const std::shared_ptr<BVHNode>& left, const std::shared_ptr<BVHNode>& right): box(box), left(left), right(right) {}

    /**
     * A recursive function that builds the BVH tree from the input vector of nodes
     * It returns a pointer to the root of the current subtree (or the overall root on the outermost call)
     * It uses the SAH function to divide the nodes to the left and right child
     * When called inside an omp parallel block, it uses OpenMP task parallelism to speed up the construction
     *
     * @param[in]   nodes   Nodes to build the current subtree from
     */
    static std::shared_ptr<BVHNode> BuildBVH(std::vector<std::shared_ptr<BVHNode>>& nodes);
    static SAHResult SAHSplitNodes(std::vector<std::shared_ptr<BVHNode>>& nodes, const BoundingBox& parent_box);
};

/**
 * @class CompactBVHNode
 * Represents a node of a compacted BVH tree
 * Compacted BVH tree is a tree compacted to be stored in an array
 * Nodes do not store pointers, but indices of its children
 */
class CompactBVHNode {
public:

    /**
     * @enum ChildType
     * Represents the type of child node - right or left
     */
    enum ChildType {
        left,
        right
    };

    const BoundingBox box;      /**< The bounding box of the BVH node, it contains a face if left and right children are SIZE_MAX */
    size_t left_idx;            /**< Index of the left child of the BVH node */
    size_t right_idx;           /**< Index of the right child of the BVH node */

    /**
     * Basic Constructor that just assigns all the input fields
     *
     * @param[in]   box         The bounding box of the node
     * @param[in]   left_idx    The index of the left child of the node
     * @param[in]   right_idx   The index of the right child of the node
     */
    CompactBVHNode(const BoundingBox& box, const size_t left_idx, const size_t right_idx): box(box), left_idx(left_idx), right_idx(right_idx) {}

};

/**
 * @struct SAHResult
 * A helper struct that returns the result of SAH, it stores the left and right children of the current node based on SAH
 */
struct SAHResult {
    std::vector<std::shared_ptr<BVHNode>> left;     /**< Left children nodes */
    std::vector<std::shared_ptr<BVHNode>> right;    /**< Right children nodes */

    /**
     * Basic empty constructor, sets both vector to empty
     */
    SAHResult() {
        left = {};
        right = {};
    }
};

/**
 * @struct SAHBest
 * A helper struct for SAH calculation, it holds the current best cost and the splitting index and axis that create it
 */
struct SAHBest {
    float cost;     /**< The cost of the split */
    int index;      /**< The index of the split */
    int axis;       /**< The axis of the split */

    /**
     * Basic empty constructor to create the first "best" to compare to
     */
    SAHBest() {
        cost = INFINITY;
        index = 0;
        axis = 0;
    }
};


#endif //RAYTRACER_BVH_NODE_HPP