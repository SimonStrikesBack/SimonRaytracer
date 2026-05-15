/**
 * @file scene.cpp
 * @author Simon Tanev
 * @brief Implementation file for scene.hpp, see the header file for descriptions
 */

#define TINYOBJLOADER_IMPLEMENTATION
#include "./include/tiny_obj_loader.h"
#include "geomath.hpp"
#include <json.hpp>
#include <fstream>
#include <vector>
#include <cassert>
#include <string>
#include "scene.hpp"
#include "point_light.hpp"
#include "area_light.hpp"
#include "bvh_node.hpp"
#include "TBRDF.h"
#include <stack>
#include <omp.h>
#include <chrono>
using json = nlohmann::json;

Scene::Scene(const char* config_path, const char* scene_path, const char* scene_name) {
    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream config(config_path);
    json data = json::parse(config);

    const auto& json_lights = data["lights"];
    bool fast_bvh = data.value("bvh_build_opt", true);

    float scene_scale = data.value("scale", 1.0f);
    for(const auto& light : json_lights){
        PointLight l = PointLight(vec3(light.value("position", std::vector<float>())) / scene_scale, light.value("color", std::vector<float>()));
        lights.emplace_back(std::make_shared<PointLight>(l));
    }

    std::string input_path = std::string(scene_path) + std::string(scene_name);
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> tiny_materials;

    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &tiny_materials, &err, input_path.c_str(), scene_path);

    //if (!err.empty()) {
    //    std::cerr << err << std::endl;
    //}

    if (!ret) {
        exit(1);
    }

    std::vector<std::shared_ptr<Material>> materials = {};

    for (auto& tiny_material : tiny_materials) {
        vec3 diffuse(tiny_material.diffuse);
        vec3 specular(tiny_material.specular);
        vec3 emissive(tiny_material.emission);
        float shininess(tiny_material.shininess);

        vec3 transmittance(tiny_material.transmittance);
        float ior(tiny_material.ior);
        float dissolve(tiny_material.dissolve);
        std::string name(tiny_material.name);
        std::string diffuse_texture_name(tiny_material.diffuse_texname);
        std::string normal_texture_name(tiny_material.normal_texname);

        std::shared_ptr<Material> material = std::make_shared<Material>(diffuse, specular, emissive, shininess, transmittance, ior, dissolve, name, diffuse_texture_name, normal_texture_name);
        materials.emplace_back(material);
        MaterialMap.insert({material->name, material});

        if (not diffuse_texture_name.empty()) {
            char texpath[128];
            strcpy(texpath, scene_path);
            strcat(texpath, diffuse_texture_name.c_str());
            std::shared_ptr<Texture> tex = std::make_shared<Texture>(texpath);

            TextureMap.insert({diffuse_texture_name, tex});
        }
        if (not normal_texture_name.empty()) {
            char texpath[128];
            strcpy(texpath, scene_path);
            strcat(texpath, normal_texture_name.c_str());
            std::shared_ptr<Texture> tex = std::make_shared<Texture>(texpath);

            TextureMap.insert({normal_texture_name, tex});
        }
    }

    const auto json_brdf_override = data["brdf_override"];
    char brdf_path[128];
    strcpy(brdf_path,data.value("brdf_path", "../").c_str());

    for (const auto &override : json_brdf_override) {
        for (const auto& [key, value] : override.items()) {
            if (BRDFMap.find(value) != BRDFMap.end()) {
                MaterialMap.at(key)->brdf = BRDFMap.at(value);
                MaterialMap.at(key)->brdf_name = value;
                continue;
            }
            char full_path[128];
            strcpy(full_path, brdf_path);
            strcat(full_path, value.get<std::string>().c_str());

            std::shared_ptr<BRDF> brdf = std::make_shared<BRDF>();
            brdf->load_brdf(full_path);

            MaterialMap.at(key)->brdf = brdf;
            MaterialMap.at(key)->brdf_name = value;
            BRDFMap.insert({value, brdf});
        }
    }

    std::vector<std::shared_ptr<BVHNode>> nodes = {};

    //omp_set_num_threads(1);
    //omp_set_num_threads(omp_get_max_threads());

    // Loop over shapes

        for (size_t s = 0; s < shapes.size(); s++) {
            std::vector<std::shared_ptr<triangle>> faces = {};

            size_t mesh_vertices_amount = shapes[s].mesh.num_face_vertices.size();
            assert(mesh_vertices_amount > 0);

            // Loop over faces(polygon)
            for (int f = 0; f < mesh_vertices_amount; f++) {
                constexpr size_t fv = 3;
                size_t index_offset = f * fv;
                size_t material_idx = shapes[s].mesh.material_ids[f];

                std::vector<vertex> face_vertices = {};
                bool generate_normals = false;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    const vec3 position = vec3(attrib.vertices[3*size_t(idx.vertex_index)+0], attrib.vertices[3*size_t(idx.vertex_index)+1], attrib.vertices[3*size_t(idx.vertex_index)+2]) / scene_scale;
                    vertex vert = vertex(position);
                    if (idx.normal_index >= 0) {
                        const vec3 normal = vec3(attrib.normals[3*size_t(idx.normal_index)+0],attrib.normals[3*size_t(idx.normal_index)+1],attrib.normals[3*size_t(idx.normal_index)+2]);
                        vert.normal = normal;
                    }
                    else generate_normals = true;

                    if (idx.texcoord_index >= 0) {
                        const vec2 uv = vec2(attrib.texcoords[2*size_t(idx.texcoord_index)+0], attrib.texcoords[2*size_t(idx.texcoord_index)+1]);
                        vert.uv = uv;
                    }
                    face_vertices.push_back(vert);
                }
                assert(face_vertices.size() > 2);

                if (generate_normals) {
                    vec3 a = face_vertices[1].position - face_vertices[0].position;
                    vec3 b = face_vertices[2].position - face_vertices[0].position;
                    const vec3 normal = cross(a, b).normalize();
                    face_vertices[0].normal = normal;
                    face_vertices[1].normal = normal;
                    face_vertices[2].normal = normal;
                }

                triangle face(face_vertices[0], face_vertices[1], face_vertices[2], materials[material_idx]);

                if (not face.material->diffuse_texture_name.empty()) {
                    face.material->texture = TextureMap.at(face.material->diffuse_texture_name);
                }
                if (not face.material->normal_texture_name.empty()) {
                    face.material->normal_map = TextureMap.at(face.material->normal_texture_name);
                }

                std::shared_ptr<triangle> face_ptr = std::make_shared<triangle>(face);

                faces.emplace_back(face_ptr);
                nodes.emplace_back(std::make_shared<BVHNode>(BoundingBox(face_ptr), nullptr, nullptr));

                if (materials[material_idx]->emissive != vec3(0.0f)) {
                    // create area light
                    AreaLight l = AreaLight(face, materials[material_idx]->emissive);

                    lights.emplace_back(std::make_shared<AreaLight>(l));
                }
            }

            std::shared_ptr<Object> object = std::make_shared<Object>(faces);

            objects.emplace_back(object);
        }


    auto middle = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seconds = middle - start;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Scene Conversion Load Time: " << seconds.count() << " seconds\n";

    // Build top-down BVH Tree
    std::shared_ptr<BVHNode> root = nullptr;

    #pragma omp parallel if(fast_bvh)
    {
        #pragma omp single
        {
            root = BVHNode::BuildBVH(nodes);
            #pragma omp taskwait
        }
    }

    auto middle2 = std::chrono::high_resolution_clock::now();
    seconds = middle2 - middle;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "BVH Build Time: " << seconds.count() << " seconds\n";

    // Build compact BVH Tree
    std::stack<std::shared_ptr<BVHNode>> DFSStack;
    std::stack<size_t> index_stack;
    std::stack<CompactBVHNode::ChildType> child_stack;

    DFSStack.emplace(root);
    index_stack.emplace(SIZE_MAX);

    compact_bvh.clear();

    while (!DFSStack.empty()) {
        auto v = DFSStack.top();
        size_t idx = index_stack.top();
        DFSStack.pop();
        index_stack.pop();

        if (idx != SIZE_MAX) {
            CompactBVHNode::ChildType type = child_stack.top();
            child_stack.pop();
            if (type == CompactBVHNode::left) compact_bvh[idx].left_idx = compact_bvh.size();
            else if (type == CompactBVHNode::right) compact_bvh[idx].right_idx = compact_bvh.size();
        }

        CompactBVHNode node(v->box, SIZE_MAX, SIZE_MAX);

        if (v->right != nullptr) {
            DFSStack.emplace(v->right);
            child_stack.emplace(CompactBVHNode::right);
            index_stack.emplace(compact_bvh.size());
        }
        if (v->left != nullptr) {
            DFSStack.emplace(v->left);
            child_stack.emplace(CompactBVHNode::left);
            index_stack.emplace(compact_bvh.size());
        }

        compact_bvh.emplace_back(node);
    }

    auto end = std::chrono::high_resolution_clock::now();
    seconds = end - middle2;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "BVH Conversion Time: " << seconds.count() << " seconds\n";

    seconds = end - start;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Overall Scene Load Time: " << seconds.count() << " seconds\n";

}