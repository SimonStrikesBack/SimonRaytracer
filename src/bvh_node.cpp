/**
 * @file bvh_node.cpp
 * @author Simon Tanev
 * @brief Implementation file for bvh_node.hpp, see the header file for descriptions
 */

#include "bvh_node.hpp"
#include <algorithm>
#include <ranges>
#include <cassert>
#include <omp.h>

//int BVHNode::parallelism_threshold = omp_get_max_threads() * omp_get_max_threads() * omp_get_max_threads() * omp_get_max_threads();

SAHResult BVHNode::SAHSplitNodes(std::vector<std::shared_ptr<BVHNode>> &nodes, const BoundingBox& parent_box) {
    assert(nodes.size() > 1);

    SAHBest best = SAHBest();
    best.cost = INFINITY;

    for (int axis = 0; axis < 3; axis++) {
        std::ranges::sort(nodes,
                          [axis](const std::shared_ptr<BVHNode>& a, const std::shared_ptr<BVHNode>& b) {
                              const float ca = a->box.centroid[axis];
                              const float cb = b->box.centroid[axis];
                              return ca < cb;
                          });

        std::vector<std::pair<float, float>> split_areas;

        float accumulate = 0;
        for (size_t i = 0; i < nodes.size() - 1; i++) {
            accumulate += nodes[i]->box.surface_area;
            split_areas.emplace_back(accumulate, 0);
        }
        accumulate = 0;
        for (size_t i = nodes.size() - 1; i > 0; i--) {
            accumulate += nodes[i]->box.surface_area;
            split_areas[i-1].second = accumulate;
        }

        const float parent_area = parent_box.surface_area;
        constexpr float traversal_cost = 1.0f; // -O3 creates 51 lines of assembly, see assembly.asm in test_assembly folder
        constexpr float primitive_cost = 1.9f; // -O3 creates 98 lines of longest branch assembly, see assembly.asm in test_assembly folder

        /*if (nodes.size() > parallelism_threshold) {
            #pragma omp parallel
            {
                SAHBest local_best = SAHBest();

                #pragma omp for schedule(dynamic)
                for (int i = 0; i < split_areas.size(); i++) {
                    const float left_probability = split_areas[i].first / parent_area;
                    const float right_probability = split_areas[i].second / parent_area;

                    const float cost = traversal_cost + ((left_probability * static_cast<float>(i + 1) * primitive_cost) + (right_probability * static_cast<float>(split_areas.size() - i) * primitive_cost));


                    if (cost < local_best.cost) {
                        local_best.cost = cost;
                        local_best.index = i;
                        local_best.axis = axis;
                    }
                }

                #pragma omp critical
                {
                    if (local_best.cost < best.cost) {
                        best = local_best;
                    }
                }
            }
        }*/
        //else {
            for (int i = 0; i < split_areas.size(); i++) {
                const float left_probability = split_areas[i].first / parent_area;
                const float right_probability = split_areas[i].second / parent_area;

                const float cost = traversal_cost + ((left_probability * static_cast<float>(i + 1) * primitive_cost) + (right_probability * static_cast<float>(split_areas.size() - i) * primitive_cost));

                if (cost < best.cost) {
                    best.cost = cost;
                    best.index = i;
                    best.axis = axis;
                }
            }
        //}
    }

    int axis = best.axis;

    std::ranges::sort(nodes,
                          [axis](const std::shared_ptr<BVHNode>& a, const std::shared_ptr<BVHNode>& b) {
                              const float ca = a->box.centroid[axis];
                              const float cb = b->box.centroid[axis];
                              return ca < cb;
                          });

    SAHResult result = SAHResult();

    for (size_t j = 0; j < nodes.size(); j++) {
        if (j <= best.index) result.left.emplace_back(nodes[j]);
        else result.right.emplace_back(nodes[j]);
    }
    return result;
}

std::shared_ptr<BVHNode> BVHNode::BuildBVH(std::vector<std::shared_ptr<BVHNode>> &nodes) {
    if (nodes.size() == 1) {
        std::shared_ptr<BVHNode> node = std::make_shared<BVHNode>(nodes[0]->box, nullptr, nullptr);
        return node;
    }

    std::vector<BoundingBox> boxes = {};
    for (const auto& node : nodes) {
        boxes.emplace_back(node->box);
    }

    BoundingBox box = BoundingBox(boxes);
    SAHResult result = SAHSplitNodes(nodes, box);

    std::shared_ptr<BVHNode> child_left = nullptr;
    std::shared_ptr<BVHNode> child_right = nullptr;

    #pragma omp task shared(child_left) //if(result.left.size() <= parallelism_threshold)
    {
        child_left = BuildBVH(result.left);
    }

    #pragma omp task shared(child_right) //if(result.right.size() <= parallelism_threshold)
    {
        child_right = BuildBVH(result.right);
    }

    #pragma omp taskwait
    std::shared_ptr<BVHNode> node = std::make_shared<BVHNode>(box, child_left, child_right);
    return node;
}
