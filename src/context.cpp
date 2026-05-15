/**
 * @file context.cpp
 * @author Simon Tanev
 * @brief Implementation file for context.hpp, see the header file for descriptions
 */
#include "context.hpp"
#include <iostream>
#include <json.hpp>
#include <fstream>
#include <cmath>
#include <cstring>
#include "geomath.hpp"
using json = nlohmann::json;

Context::Context(const char *config) {
    std::ifstream file(config);
    json data = json::parse(file);

    scene_scale = data.value("scale", 1.0);
    width = data.value("width", 600);
    height = data.value("height", 600);
    origin = vec3(data.value("pos", std::vector<float>())) / scene_scale;
    direction = vec3(data.value("dir", std::vector<float>())).normalize();
    up = vec3(data.value("up", std::vector<float>())).normalize();
    right = cross(direction, up).normalize();
    FOV = data.value("fov", 0.6);
    strcpy(scene_path, data.value("scene_path", "../../CornellBox/").c_str());
    strcpy(scene_name,data.value("scene_name", "CornellBox-Original.obj").c_str());
    strcpy(output_path,data.value("output_path", "../").c_str());
    strcpy(brdf_path, data.value("brdf_path", "../").c_str());
    area_light_samples = data.value("area_light_samples", 32);
    max_recursion = data.value("max_recursion", 8);
    BVH = data.value("BVH", true);
    reflection = data.value("reflection", true);
    refraction = data.value("refraction", true);
    shadows = data.value("shadows", true);
    normal_mapping = data.value("normal_mapping", true);
    expensive_rays = data.value("expensive_rays", true);
    expensive_threshold = data.value("expensive_threshold", 6);
    std::string parallel = data.value("Parallelism", "CPU");
    if (parallel == "CPU") {
        parallelism = CPU;
    }
    else if (parallel == "GPU") {
        parallelism = GPU;
    }
    else parallelism = NONE;

    passes = {};
    passes.emplace("combined", Buffer("output.ppm", RGB, false));
    passes.emplace("depth", Buffer("output_depth.ppm", BW, false));
    passes.emplace("diffuse", Buffer("output_diffuse.ppm", RGB, false));
    passes.emplace("specular", Buffer("output_specular.ppm", RGB, false));
    passes.emplace("emissive", Buffer("output_emissive.ppm", RGB, false));
    passes.emplace("material_diffuse", Buffer("output_material_diffuse.ppm", RGB, false));
    passes.emplace("material_specular", Buffer("output_material_specular.ppm", RGB, false));
    passes.emplace("reflection", Buffer("output_reflection.ppm", RGB, false));
    passes.emplace("refraction", Buffer("output_refraction.ppm", RGB, false));

    const auto data_passes = data["passes"];
    if (data_passes.value("combined", true)) passes.at("combined").output = true;
    if (data_passes.value("depth", false)) passes.at("depth").output = true;
    if (data_passes.value("diffuse", false)) passes.at("diffuse").output = true;
    if (data_passes.value("specular", false)) passes.at("specular").output = true;
    if (data_passes.value("emissive", false)) passes.at("emissive").output = true;
    if (data_passes.value("material_diffuse", false)) passes.at("material_diffuse").output = true;
    if (data_passes.value("material_specular", false)) passes.at("material_specular").output = true;
    if (data_passes.value("reflection", false)) passes.at("reflection").output = true;
    if (data_passes.value("refraction", false)) passes.at("refraction").output = true;

    //float t = (float(width) / 2.0f) / tan(FOV / 2.0f);
    float t = 1.0f;
    g_w = 2 * t * tan(FOV / 2.0f);

    g_h = g_w * float(height) / float(width);

    q_w = (g_w / float(width - 1)) * right;

    q_h = (g_h / float(height - 1)) * up;

    p_00 = t * direction - (g_w / 2) * right + (g_h / 2) * up;
}

void Context::PrintPasses() const {
    std::cout << "Rendering Passes:" << std::endl;
    for (auto &[fst, snd] : passes) {
        std::cout << "  " << fst << std::boolalpha << ": " << snd.output << std::endl;
    }
}

std::string Context::ParallelismToString(const Parallelism parallelism) {
    if (parallelism == NONE) return "None";
    if (parallelism == CPU) return "CPU";
    return "GPU";
}

std::string Context::ParallelismToString() const {
    if (parallelism == NONE) return "None";
    if (parallelism == CPU) return "CPU";
    return "GPU";
}

void Context::PrintRenderInfo() const {
    std::cout << "Render Size: " << width << "x" << height << std::endl;
    std::cout << "Area light samples: " << area_light_samples << ", Max Recursion Depth: " << max_recursion << std::endl;
    std::cout << std::boolalpha << "Shadows: " << shadows << ", Reflection: " << reflection << ", Refraction: " << refraction << std::endl;
    std::cout << std::boolalpha << "Use BVH: " << BVH << ", Parallelism Type: " << ParallelismToString() << std::endl;
}

void Context::PrintHelp() {
    std::cout << "+--------------------------------------------------Simon's Raytracer--------------------------------------------------+" << std::endl;
    std::cout << "| App usage options:                                                                                                  |" << std::endl;
    std::cout << "|     --help:                             Print help string. You have figured that out already though, right?         |" << std::endl;
    std::cout << "|     <path_to_config>:                   Try to load config from the path to the file (See config usage below)       |" << std::endl;
    std::cout << "+---------------------------------------------------------------------------------------------------------------------+" << std::endl;
    std::cout << "| Config file fields (JSON):                                                                                          |" << std::endl;
    std::cout << "|     width: int                          Width of the rendered image in pixels                                       |" << std::endl;
    std::cout << "|     height: int                         Height of the rendered image in pixels                                      |" << std::endl;
    std::cout << "|     pos: array[float]                   Position of camera in scene, 3 values (x,y,z)                               |" << std::endl;
    std::cout << "|     up: array[float]                    Up vector, 3 values (x,y,z)                                                 |" << std::endl;
    std::cout << "|     dir: array[float]                   Direction (Front) vector, 3 values (x,y,z)                                  |" << std::endl;
    std::cout << "|     FOV: float                          FOV (Field of View)                                                         |" << std::endl;
    std::cout << "|     scene_path: string                  Path to the folder that includes the scene (with '/' suffix)                |" << std::endl;
    std::cout << "|     scene_name: string                  Name of the .obj scene file (with '.obj' suffix)                            |" << std::endl;
    std::cout << "|     output_path: string                 Path to the folder where renders will be saved to (with '/' suffix)         |" << std::endl;
    std::cout << "|     area_light_samples: int             Number of samples to create when calculating area lights                    |" << std::endl;
    std::cout << "|     max_recursion: int                  Number of ray bounces i.e. ray trace recursion calls                        |" << std::endl;
    std::cout << "|     scale: int                          Factor to scale down the scene by (makes difference in light attenuation)   |" << std::endl;
    std::cout << "|     BVH: bool                           Use BVH to accelerate ray traversal in scene                                |" << std::endl;
    std::cout << "|     bvh_build_opt: bool                 Use optimizations to make BVH building faster                               |" << std::endl;
    std::cout << "|     Parallelism: string                 Type of Parallelism to use ('CPU' | 'GPU' | 'NONE')                         |" << std::endl;
    std::cout << "|     reflection: bool                    Calculate reflected ray shading                                             |" << std::endl;
    std::cout << "|     refraction: bool                    Calculate refracted ray shading                                             |" << std::endl;
    std::cout << "|     shadows: bool                       Test light visibility from point                                            |" << std::endl;
    std::cout << "|     normal_mapping: bool                Bend normals according to normal maps (if present)                          |" << std::endl;
    std::cout << "|     expensive_rays: bool                Perform expensive ray optimization on GPU rendering                         |" << std::endl;
    std::cout << "|     expensive_threshold: int            Max recursion depth for a ray to not be considered expensive (GPU only)     |" << std::endl;
    std::cout << "|     lights: array[JSONObject]           Array of point lights to use in scene (See light definition below)          |" << std::endl;
    std::cout << "|         position: array[float]          Position of point light in scene, 3 values (x,y,z)                          |" << std::endl;
    std::cout << "|         color: array[float]             Color of point light, 3 values (r,g,b)                                      |" << std::endl;
    std::cout << "|     passes: JSONObject                  Dictionary of which render passes to save (See available passes below)      |" << std::endl;
    std::cout << "|         combined: bool                  Final combined render pass                                                  |" << std::endl;
    std::cout << "|         depth: bool                     Depth Buffer render pass                                                    |" << std::endl;
    std::cout << "|         diffuse: bool                   Shaded diffuse render pass                                                  |" << std::endl;
    std::cout << "|         specular: bool                  Shaded specular render pass                                                 |" << std::endl;
    std::cout << "|         emissive: bool                  Shaded emissive render pass                                                 |" << std::endl;
    std::cout << "|         material_diffuse: bool          Material diffuse color render pass                                          |" << std::endl;
    std::cout << "|         material_specular: bool         Material specular color render pass                                         |" << std::endl;
    std::cout << "|         reflection: bool                Reflection render pass                                                      |" << std::endl;
    std::cout << "|         refraction: bool                Refraction render pass                                                      |" << std::endl;
    std::cout << "|     brdf_path: string                   Path to the folder that includes BRDF materials (.bin files)                |" << std::endl;
    std::cout << "|     brdf_override: array[JSONObject]    Array of material overrides to use BRDF instead of standard diffuse         |" << std::endl;
    std::cout << "|         <material_name>: string         Mapping of material name to a string that points to a BRDF file             |" << std::endl;
    std::cout << "+---------------------------------------------------------------------------------------------------------------------+" << std::endl;
}

void Context::NormalizeDepthBuffer() const {
    // Normalize depth buffer for output
    if (passes.at("depth").output) {
        float max = -INFINITY;
        for (size_t i = 0; i < width * height; i++) {
            if (passes.at("depth").data[i] != INFINITY) {
                max = max < passes.at("depth").data[i] ? passes.at("depth").data[i] : max;
            }
        }
        for (size_t i = 0; i < width * height; i++) {
            if (passes.at("depth").data[i] == INFINITY) {
                passes.at("depth").data[i] = 0.0f;
            }
            else {
                passes.at("depth").data[i] = 255.0f - (passes.at("depth").data[i] / max) * 200.0f;
            }
        }
    }
}
