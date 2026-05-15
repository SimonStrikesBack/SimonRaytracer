/**
 * @file gpu_scene.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Scene class
 */

#pragma once
#include <cuda_runtime_api.h>
#include <ranges>

#include "gpu_area_light.cuh"
#include "gpu_bvh_node.cuh"
#include "gpu_point_light.cuh"
#include "../scene.hpp"
#include "gpu_triangle.cuh"

/**
 * @class gpu_Scene
 *
 * GPU compatible Scene class
 */
class gpu_Scene {
public:
    // Scene Geometry
    gpu_triangle* geometry;                 /**< Scene geometry as a simple array */
    size_t geometry_size;                   /**< Size of scene geometry array */

    // Lights
    gpu_AreaLight* area_lights;             /**< Scene area lights array */
    size_t area_lights_size;                /**< Size of scene area lights array */

    gpu_PointLight* point_lights;           /**< Scene point lights array */
    size_t point_lights_size;               /**< Size of scene point lights array */

    // Scene Geometry in BVH
    gpu_CompactBVHNode* compact_BVH;        /**< Scene geometry as a compacted BVH */
    size_t compact_BVH_size;                /**< Size of scene geometry BVH */

    // Materials
    gpu_Material* materials;                /**< Scene materials array */
    size_t materials_size;                  /**< Size of scene materials array */

    // Textures
    gpu_Texture* textures;                  /**< Scene textures array */
    size_t textures_size;                   /**< Size of scene textures array */

    // BRDFs
    gpu_BRDF* brdfs;                        /**< Scene BRDF materials array */
    size_t brdfs_size;                      /**< Size of scene BRDF materials array */

    /**
     * Constructor from a CPU Scene
     *
     * @param[in] scene     CPU Scene
     */
    __host__
    gpu_Scene(const Scene& scene) {
        // Nullptr initialize
        geometry = nullptr;
        area_lights = nullptr;
        point_lights = nullptr;
        compact_BVH = nullptr;
        materials = nullptr;
        textures = nullptr;
        brdfs = nullptr;

        // Convert Scene Lights
        area_lights_size = 0;
        point_lights_size = 0;
        for (auto& l : scene.lights) {
            if (l->Type() == "Area") area_lights_size++;
            else if (l->Type() == "Point") point_lights_size++;
        }

        cudaMallocManaged(reinterpret_cast<void **>(&area_lights), area_lights_size * sizeof(gpu_AreaLight));
        cudaMallocManaged(reinterpret_cast<void **>(&point_lights), point_lights_size * sizeof(gpu_PointLight));

        for (size_t i = 0, a = 0, p = 0; i < scene.lights.size(); i++) {
            if (scene.lights[i]->Type() == "Area") {area_lights[a] = gpu_AreaLight(*static_pointer_cast<const AreaLight>(scene.lights[i])); a++;}
            else if (scene.lights[i]->Type() == "Point") {point_lights[p] = gpu_PointLight(*static_pointer_cast<const PointLight>(scene.lights[i])); p++;}
        }

        // Convert Scene Textures
        textures_size = scene.TextureMap.size();
        cudaMallocManaged(reinterpret_cast<void **>(&textures), textures_size * sizeof(gpu_Texture));

        std::unordered_map<std::string, size_t> texture_idxs;

        size_t i = 0;
        for (const auto& [fst, snd]: scene.TextureMap) {
            new (&textures[i]) gpu_Texture(*snd);
            texture_idxs.insert({fst, i});
            i++;
        }

        // Convert Scene BRDFs
        brdfs_size = scene.BRDFMap.size();
        cudaMallocManaged(reinterpret_cast<void **>(&brdfs), brdfs_size * sizeof(gpu_BRDF));

        std::unordered_map<std::string, size_t> brdf_idxs;
        i = 0;

        for (const auto& [fst, snd] : scene.BRDFMap) {
            new (&brdfs[i]) gpu_BRDF(*snd);
            brdf_idxs.insert({fst, i});
            i++;
        }

        // Convert Scene Materials
        materials_size = scene.MaterialMap.size();
        cudaMallocManaged(reinterpret_cast<void **>(&materials), materials_size * sizeof(gpu_Material));


        std::unordered_map<std::string, size_t> material_idxs;
        i = 0;

        for (const auto& [fst, snd] : scene.MaterialMap) {
            materials[i] = gpu_Material(*snd);
            material_idxs.insert({fst, i});
            if (snd->texture != nullptr) materials[i].texture = &(textures[texture_idxs.at(snd->diffuse_texture_name)]);
            if (snd->normal_map != nullptr) materials[i].normal_map = &(textures[texture_idxs.at(snd->normal_texture_name)]);
            if (snd->brdf != nullptr) {
                materials[i].brdf = &(brdfs[brdf_idxs.at(snd->brdf_name)]);
            }
            i++;
        }

        // Convert Scene Objects (Simple)
        geometry_size = 0;
        for (const auto& object : scene.objects) {
            for (const auto& face : object->faces) geometry_size++;
        }

        cudaMallocManaged(reinterpret_cast<void **>(&geometry), geometry_size * sizeof(gpu_triangle));

        i = 0;
        for (const auto& object : scene.objects) {
            for (const auto& face : object->faces) {
                geometry[i] = gpu_triangle(*face);
                geometry[i].material = &materials[material_idxs.at(face->material->name)];

                i++;
            }
        }

        // Convert Scene Compact BVH
        compact_BVH_size = scene.compact_bvh.size();
        cudaMallocManaged(reinterpret_cast<void **>(&compact_BVH), compact_BVH_size * sizeof(gpu_CompactBVHNode));

        i = 0;
        for (const auto& node: scene.compact_bvh) {
            compact_BVH[i] = gpu_CompactBVHNode(node);
            if (compact_BVH[i].box.has_face) compact_BVH[i].box.face.material = &materials[material_idxs.at(node.box.face->material->name)];
            i++;
        }

    }

    /**
     * Destructor of the class
     * It calls free functions on the CUDA memory stored
     */
    __host__
    ~gpu_Scene() {
        for (size_t i = 0; i < textures_size; i++) {
            textures[i].~gpu_Texture();
        }
        for (size_t i = 0; i < brdfs_size; i++) {
            brdfs[i].~gpu_BRDF();
        }
        cudaFree(geometry);
        cudaFree(area_lights);
        cudaFree(point_lights);
        cudaFree(compact_BVH);
        cudaFree(materials);
        cudaFree(textures);
    }

};