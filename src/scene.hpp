/**
 * @file scene.hpp
 * @author Simon Tanev
 * @brief File containing Scene class
 */

#ifndef RAYTRACER_SCENE_HPP
#define RAYTRACER_SCENE_HPP
#pragma once
#include <vector>
#include <memory>
#include "object.hpp"
#include "light.hpp"
#include "bvh_node.hpp"
#include "texture.hpp"
#include "include/TBRDF.h"
#include <unordered_map>

/**
 * @class Scene
 *
 * Scene holds the scene geometry, other objects and their data
 */
class Scene {
public:
    std::vector<std::shared_ptr<Object>> objects = {};                                  /**< Scene geometry, stored as a simple vector of objects */
    std::vector<std::shared_ptr<Light>> lights = {};                                    /**< Scene lights */
    std::vector<CompactBVHNode> compact_bvh = {};                                       /**< Scene geometry, stored as a compacted BVH of faces */
    std::unordered_map<std::string, std::shared_ptr<Material>> MaterialMap = {};        /**< Map of materials and their names */
    std::unordered_map<std::string, std::shared_ptr<Texture>> TextureMap = {};          /**< Map of textures and their names */
    std::unordered_map<std::string, std::shared_ptr<BRDF>> BRDFMap = {};                /**< Map of BRDF materials and their names */

    /**
     * Loads the scene based on the config at config path and scene files at scene path
     *
     * It uses tinyobj library to parse the obj and mtl files and then converts the geometry to the custom classes
     * It uses TBRDF library to load BRDF materials
     * It then builds the BVH tree and converts it to the compacted version
     * BVH tree construction may be parallelized using openmp if toggled on in context
     *
     * @param[in] config_path   path to the config file
     * @param[in] scene_path    path to the scene directory
     * @param[in] scene_name    name of the scene in the directory
     */
    Scene(const char* config_path, const char* scene_path, const char* scene_name);

};


#endif //RAYTRACER_SCENE_HPP
