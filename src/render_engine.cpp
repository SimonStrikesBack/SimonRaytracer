/**
 * @file render_engine.cpp
 * @author Simon Tanev
 * @brief Implementation file for render_engine.hpp, see the header file for descriptions
 */

#include "render_engine.hpp"
#include "geomath.hpp"
#include "ray.hpp"
#include <unordered_map>
#include <cmath>
#include <string>
#include "point_light.hpp"
#include "area_light.hpp"
#include <random>
#include <thread>
#include <queue>
#include <ranges>
#include <utility>
#include <stack>
#include "mat3.hpp"

thread_local std::random_device RenderEngine::r;
thread_local std::minstd_rand RenderEngine::engine;
thread_local std::uniform_real_distribution<float> RenderEngine::distribution;

void RenderEngine::RenderXRange(const int from, const int to) const {
    engine = std::minstd_rand(r());
    distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);

    std::unordered_map<std::string, vec3> output_colors;
    for (const auto &fst: ctx->passes | std::views::keys) {
        if (fst != "depth") output_colors.emplace(fst, vec3());
    }

    for (int i = from; i < to; ++i){
        for (int j = 0; j < ctx->height; ++j){
            const int x = i;
            const int y = j;

            ctx->passes.at("depth").data[(y * ctx->width + x)] = INFINITY;

            // Ray init
            vec3 p_xy = ctx->p_00 + static_cast<float>(x) * ctx->q_w - static_cast<float>(y) * ctx->q_h;
            Ray ray(ctx->origin, p_xy.normalize());

            // TraceRay
            TracePrimaryRay(ray, x, y, output_colors);

            // Write to buffers
            for (auto &[fst, snd] : output_colors) {
                if (ctx->passes.at(fst).output) {
                    ctx->passes.at(fst).data[3 * (y * ctx->width + x) + 0] = clamp(snd.x, 0.0f, 255.0f);
                    ctx->passes.at(fst).data[3 * (y * ctx->width + x) + 1] = clamp(snd.y, 0.0f, 255.0f);
                    ctx->passes.at(fst).data[3 * (y * ctx->width + x) + 2] = clamp(snd.z, 0.0f, 255.0f);
                }
            }
        }
    }
}

void RenderEngine::RenderSequential(std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end) const {
    constexpr int num_jobs = 10;
    const int range = ctx->width / num_jobs;
    // Generate Jobs
    std::cout << "Rendering Sequentially on CPU" << std::endl;
    std::cout << "Progress: 0.0%" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_jobs; i++) {
        const int from = i * range;
        int to = (i + 1) * range;
        if (i + 1 == num_jobs) to = ctx->width;

        RenderXRange(from, to);
        std::cout << "\033[F\033[K";
        std::cout << "Progress: " << (static_cast<float>(i + 1) / static_cast<float>(num_jobs)) * 100.0f << "%" << std::endl;
    }
    std::cout << "\033[F\033[K";
    std::cout << "Progress: 100%" << std::endl;

    end = std::chrono::high_resolution_clock::now();

    ctx->NormalizeDepthBuffer();
}

void RenderEngine::ThreadWorker(std::queue<std::pair<int, int>> &job_queue, std::latch& read_queue_latch) const {
    mtx.lock_shared();
    const size_t num_jobs = job_queue.size();
    mtx.unlock_shared();

    read_queue_latch.arrive_and_wait();

    while (true) {
        mtx.lock();
        if (job_queue.empty()) {mtx.unlock(); break;}

        auto [fst, snd] = job_queue.front();
        job_queue.pop();
        mtx.unlock();

        RenderXRange(fst, snd);

        stdout_mtx.lock();
        const size_t finished_threads_capture = finished_threads++;
        std::cout << "\033[F\033[K";
        std::cout << "Progress: " << (static_cast<float>(finished_threads_capture) / static_cast<float>(num_jobs)) * 100.0f << "%" << std::endl;
        stdout_mtx.unlock();

    }
}

void RenderEngine::RenderParallelCPU(std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end) const {
    const unsigned int max_threads = std::thread::hardware_concurrency();
    std::latch read_queue_latch(max_threads);

    const std::string s = max_threads > 1 ? "s" : "";
    std::cout << "Using " << max_threads << " CPU thread" << s << "." << std::endl;

    const unsigned int num_jobs = max_threads * 10;

    std::queue<std::pair<int, int>> job_queue;
    const unsigned int range = ctx->width / num_jobs;

    // Generate Jobs
    for (int i = 0; i < num_jobs; i++) {
        unsigned int from = i * range;
        unsigned int to = (i + 1) * range;
        if (i + 1 == num_jobs) to = ctx->width;

        job_queue.emplace(from, to);
    }

    std::cout << "Progress: 0.0%" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    // Init Threads
    for (int i = 0; i < max_threads; i++) {
        threads.emplace_back([this, &job_queue, &read_queue_latch]{this->ThreadWorker(job_queue, read_queue_latch);});
    }

    for (auto& thread : threads) {
        thread.join();
    }

    end = std::chrono::high_resolution_clock::now();

    std::cout << "\033[F\033[K";
    std::cout << "Progress: 100%" << std::endl;

    ctx->NormalizeDepthBuffer();
}

bool RenderEngine::VisibilityToPoint(const vec3 from, const vec3 to) const {
    const Ray ray = Ray(from, (to - from).normalize());
    float depth = INFINITY;
    float ray_length = (to - from).length();

    for(const auto& object: scene->objects){
        for(const auto& face: object->faces){
            float b1, b2;
            const float t = face->intersect(ray, true, b1, b2);
            if(t == INFINITY) continue;
            if (t < bigger_epsilon) continue;

            const float current_depth = (t * ray.dir).length();
            if(not(current_depth < depth)) continue;
            if (ray_length > current_depth + bigger_epsilon) return false;

            depth = current_depth;

        }
    }

    return (ray_length <= depth + bigger_epsilon);
}

void RenderEngine::RayIntersection(const Ray &ray, std::shared_ptr<Material> &material, vec3& normal, vec2& uv, float& depth, float& t, triangle& out_face, const bool culling, const float epsilon) const {
    for(const auto& object: scene->objects){
        for(const auto& face: object->faces){
            float b1, b2;
            const float current_t = face->intersect(ray, culling, b1, b2, epsilon);
            if (current_t < bigger_epsilon) continue;
            if(current_t == INFINITY) continue;

            const float current_depth = (current_t * ray.dir).length();
            if(not(current_depth < depth)) continue;

            material = face->material;
            depth = current_depth;
            t = current_t;

            const float b3 = 1 - (b1 + b2);
            normal = (face->a.normal * b3 + face->b.normal * b1 + face->c.normal * b2).normalize();
            uv = (face->a.uv * b3 + face->b.uv * b1 + face->c.uv * b2);
            out_face = *face;
        }
    }
}

bool RenderEngine::VisibilityToPointBVH(const vec3 from, const vec3 to) const {
    std::stack<size_t> indices;
    indices.emplace(0);

    const Ray ray = Ray(from, (to - from).normalize());
    float depth = INFINITY;
    float t = INFINITY;
    const float ray_length = (to - from).length();

    while (!indices.empty()) {
        const size_t index = indices.top();
        indices.pop();
        const auto& node = scene->compact_bvh[index];
        const float box_hit_t = node.box.RayIntersection(ray);

        if (box_hit_t < t) {
        //if (node.box.RayIntersection(ray) != INFINITY) {
            if (node.box.face == nullptr) { // not a leaf node
                if (node.right_idx == SIZE_MAX) indices.emplace(node.left_idx);         // if not a leaf, then at least one child must exist
                else if (node.left_idx == SIZE_MAX) indices.emplace(node.right_idx);    // if not a leaf, then at least one child must exist
                else { // decide on order if both children exist
                    const float left_length = (ray.origin - scene->compact_bvh[node.left_idx].box.centroid).length();
                    const float right_length = (ray.origin - scene->compact_bvh[node.right_idx].box.centroid).length();

                    size_t first_index = left_length < right_length ? node.left_idx : node.right_idx;
                    size_t second_index = left_length < right_length ? node.right_idx : node.left_idx;

                    // pushing to stack, so second index is to be pushed first
                    indices.emplace(second_index);
                    indices.emplace(first_index);
                }
            }
            else { // is a leaf node
                const auto& face = node.box.face;

                float b1, b2;
                const float current_t = face->intersect(ray, true, b1, b2);
                if(current_t == INFINITY) continue;
                if (current_t < bigger_epsilon) continue;

                const float current_depth = (current_t * ray.dir).length();
                if(not(current_depth < depth)) continue;
                if (ray_length > current_depth + bigger_epsilon) return false;

                depth = current_depth;
                t = current_t;
            }
        }
    }

    return (ray_length <= depth + bigger_epsilon);
}
//TODO Try not discarding nodes if the intersection is further than best depth
void RenderEngine::RayIntersectionBVH(const Ray &ray, std::shared_ptr<Material> &material, vec3& normal, vec2& uv, float& depth, float& t, triangle& out_face, const bool culling, const float epsilon) const{
    std::stack<size_t> indices;
    indices.emplace(0);

    while (!indices.empty()) {
        const size_t index = indices.top();
        indices.pop();

        const auto& node = scene->compact_bvh[index];
        const float box_hit_t = node.box.RayIntersection(ray);

        if (box_hit_t < t) {
        //if (node.box.RayIntersection(ray) != INFINITY){
            if (node.box.face == nullptr) { // not a leaf node
                if (node.right_idx == SIZE_MAX) indices.emplace(node.left_idx);         // if not a leaf, then at least one child must exist
                else if (node.left_idx == SIZE_MAX) indices.emplace(node.right_idx);    // if not a leaf, then at least one child must exist
                else { // decide on order if both children exist
                    const float left_length = (ray.origin - scene->compact_bvh[node.left_idx].box.centroid).length();
                    const float right_length = (ray.origin - scene->compact_bvh[node.right_idx].box.centroid).length();

                    size_t first_index = left_length < right_length ? node.left_idx : node.right_idx;
                    size_t second_index = left_length < right_length ? node.right_idx : node.left_idx;

                    // pushing to stack, so second index is to be pushed first
                    indices.emplace(second_index);
                    indices.emplace(first_index);
                }
            }
            else { // is a leaf node
                const auto& face = node.box.face;

                float b1, b2;
                const float current_t = face->intersect(ray, culling, b1, b2, epsilon);
                if (current_t < bigger_epsilon) continue;
                if(current_t == INFINITY) continue;

                const float current_depth = (current_t * ray.dir).length();
                if(not(current_depth < depth)) continue;

                material = face->material;
                depth = current_depth;
                t = current_t;

                const float b3 = 1 - (b1 + b2);
                normal = (face->a.normal * b3 + face->b.normal * b1 + face->c.normal * b2).normalize();
                uv = (face->a.uv * b3 + face->b.uv * b1 + face->c.uv * b2);
                out_face = *face;
            }
        }
    }
}

void RenderEngine::CalculateDiffuseAndSpecular(vec3 &diffuse, vec3 &specular, const Ray& ray, const std::shared_ptr<Material> &material, vec3& normal, const vec3& frag_position, const vec2& uv, vec3& diffuse_sample, const triangle& face) const {
    vec3 base_diffuse = material->diffuse;
    if (material->texture != nullptr) {
        base_diffuse = material->texture->sample_uv(uv);
    }

    diffuse_sample = base_diffuse; // assigning here, since it does not make sense after potential BRDF sampling which is not universal

    float view_elevation;
    float view_azimuth;
    float light_elevation;
    float light_azimuth;
    mat3 world_to_local;

    if (material->normal_map != nullptr && ctx->normal_mapping) {
        vec3 bitangent;
        vec3 tangent;
        //normalONB(normal, tangent, bitangent);
        uvGradientsONB(normal, tangent, bitangent, face);
        const mat3 normal_to_world = mat3(tangent, bitangent, normal);
        vec3 tex_normal = material->normal_map->sample_uv_normal(uv);
        normal = (normal_to_world * tex_normal).normalize();
    }

    if (material->brdf != nullptr) {
        vec3 bitangent;
        vec3 tangent;
        branchlessONB(normal, tangent, bitangent);
        world_to_local = mat3(tangent, bitangent, normal);

        localElevationAzimuth(-ray.dir, world_to_local, view_elevation, view_azimuth); // from the sample back to the viewer
    }

    // Per light components
    for (auto & light: scene->lights) {
        if (light->Type() == "Point") {
            const std::shared_ptr<PointLight> point = std::dynamic_pointer_cast<PointLight>(light);
            bool visible = true;
            if (ctx->shadows) visible = ctx->BVH ? VisibilityToPointBVH(frag_position, point->position) : VisibilityToPoint(frag_position, point->position);
            if (not visible) continue;

            const vec3 light_direction = (point->position - frag_position).normalize();
            const vec3 reflected_light = reflect(-light_direction, normal);

            if (material->brdf != nullptr) {
                localElevationAzimuth(light_direction, world_to_local, light_elevation, light_azimuth);

                double RGB[3];
                material->brdf->lookup_brdf_val(light_elevation, light_azimuth, view_elevation, view_azimuth, RGB);

                base_diffuse = vec3(RGB[0], RGB[1], RGB[2]);
                diffuse += base_diffuse * light->color; specular = vec3(0.0f);
            }

            else {
                diffuse += light->color * base_diffuse * std::max(dot(light_direction, normal), 0.0f);
                specular += light->color * material->specular * pow(std::max(dot(reflected_light, -ray.dir), 0.0f), material->shininess);
            }
        }
        else if (light->Type() == "Area") {
            const std::shared_ptr<AreaLight> area = std::dynamic_pointer_cast<AreaLight>(light);

            for (int i = 0; i < ctx->area_light_samples; i++) {
                const float r1 = distribution(engine);
                const float r2 = distribution(engine);
                const float u = 1.0f - sqrt(r1);
                const float v = (1.0f-r2)*sqrt(r1);

                const vec3 sampled_point = area->face.a.position + u * (area->face.b.position - area->face.a.position) + v * (area->face.c.position - area->face.a.position);
                bool visible = true;
                if (ctx->shadows) visible = ctx->BVH ? VisibilityToPointBVH(frag_position, sampled_point) : VisibilityToPoint(frag_position, sampled_point);
                if (not visible) continue;

                const vec3 d_l = (frag_position - sampled_point).normalize();
                const float intensity = (area->area * (dot(normal, d_l)) * (dot(area->normal,-d_l))) / (static_cast<float>(ctx->area_light_samples) * pow((sampled_point - frag_position).length(), 2));
                const vec3 light_color = light->color * intensity;

                const vec3 light_direction = (sampled_point - frag_position).normalize();
                const vec3 reflected_light = reflect(-light_direction, normal);

                if (material->brdf != nullptr) {
                    localElevationAzimuth(light_direction, world_to_local, light_elevation, light_azimuth);

                    double RGB[3];
                    material->brdf->lookup_brdf_val(light_elevation, light_azimuth, view_elevation, view_azimuth, RGB);

                    base_diffuse = vec3(RGB[0], RGB[1], RGB[2]);
                    diffuse += base_diffuse * light_color; specular = vec3(0.0);
                }

                else {
                    diffuse += light_color * base_diffuse * std::max(dot(light_direction, normal), 0.0f);
                    specular += light_color * material->specular * pow(std::max(dot(reflected_light, -ray.dir), 0.0f), material->shininess);
                }
            }

        }
    }

}

vec3 RenderEngine::TraceSecondaryRay(const Ray &ray, const int recursion_depth, std::stack<std::shared_ptr<Material>> material_stack) const {
    // Find intersection point and extract its properties
    vec3 normal;
    vec2 uv;
    std::shared_ptr<Material> material = nullptr;
    float depth = INFINITY;
    float t = INFINITY;

    bool culling = true;
    if (!material_stack.empty()) culling = false;

    triangle face = triangle();

    if (ctx->BVH) RayIntersectionBVH(ray, material, normal, uv, depth, t, face, culling);
    else RayIntersection(ray, material, normal, uv, depth, t, face, culling);

    if (depth == INFINITY) return {0.0f};

    // flip normal if the ray is coming from inside
    const float incoming_dot_product = dot(ray.dir, normal);
    if (incoming_dot_product > 0) normal = -normal;

    // Calculate Shading
    vec3 color_out = {0.0f, 0.0f, 0.0f};
    const vec3 frag_position = ray.origin + t * ray.dir;

    vec3 diffuse = {0.0f, 0.0f, 0.0f};
    vec3 specular = {0.0f, 0.0f, 0.0f};
    vec3 _ = {0.0f, 0.0f, 0.0f};

    CalculateDiffuseAndSpecular(diffuse, specular, ray, material, normal, frag_position, uv, _, face);

    // Reflection and Refraction
    vec3 color_reflected = vec3(0.0f);
    vec3 color_refracted = vec3(0.0f);

    if (recursion_depth < ctx->max_recursion) {
        if (ctx->reflection && material->specular != vec3(0.0f)) {
            Ray reflected_ray = Ray(frag_position, reflect(ray.dir, normal));
            color_reflected = TraceSecondaryRay(reflected_ray, recursion_depth + 1, material_stack) * material->specular;
        }

        if (ctx->refraction && material->transmittance != vec3(0.0)) {
            float refraction_first = 1;
            float ior_second = 1;
            if (material_stack.empty()) { // air -> material
                refraction_first = 1;
                ior_second = material->ior;
            }
            else if (incoming_dot_product < 0) { // material_stack.top -> material, dir and og normal go against each other
                refraction_first = material_stack.top()->refraction;
                ior_second = material->ior;
            }
            else { // material_stack.top -> material_stack.one_after_top, dir and og normal go in the same direction
                std::shared_ptr<Material> top_material = material_stack.top();
                refraction_first = top_material->refraction;
                material_stack.pop();

                if (material_stack.empty()) ior_second = 1; // material_stack.top -> air
                else ior_second = material_stack.top()->ior;

                material_stack.emplace(top_material); // material stack is to be unchanged at this point
            }

            vec3 refracted_dir = refract(-ray.dir, normal, refraction_first, ior_second);
            if (refracted_dir != vec3(0.0f)) {
                const float outcoming_dot_product = dot(refracted_dir, normal);
                if (outcoming_dot_product < 0) { // refracted ray is actually refracted, otherwise, ignore it
                    if (incoming_dot_product < 0) material_stack.emplace(material);     // The ray is going inside the mesh with material
                    else {
                        if (!material_stack.empty()) material_stack.pop(); // The ray is leaving the mesh with material at the top of material_stack, or it is leaving to air
                    }
                    color_refracted = TraceSecondaryRay(Ray(frag_position, refracted_dir), recursion_depth + 1, material_stack) * material->transmittance;
                }
            }
        }
    }

    // Writing to Buffers and Return
    color_out += (diffuse + specular + material->emissive + color_reflected + color_refracted);
    return color_out;
}

vec3 RenderEngine::TracePrimaryRay(const Ray& ray, const int x, const int y, \
                                   std::unordered_map<std::string, vec3>& output_colors) const {
    // Find intersection point and extract its properties
    vec3 normal;
    vec2 uv;
    std::shared_ptr<Material> material = nullptr;
    float depth = INFINITY;
    float t = INFINITY;

    triangle face = triangle();

    if (ctx->BVH) RayIntersectionBVH(ray, material, normal, uv, depth, t,face, true, smaller_epsilon);
    else RayIntersection(ray, material, normal, uv, depth, t, face,true, smaller_epsilon);

    if (depth == INFINITY) {
        ctx->passes.at("depth").data[(y * ctx->width + x)] = depth;
        output_colors.at("combined") = vec3(0.0f);
        output_colors.at("diffuse") = vec3(0.0f);
        output_colors.at("specular") = vec3(0.0f);
        output_colors.at("emissive") = vec3(0.0f);
        output_colors.at("material_diffuse") = vec3(0.0f);
        output_colors.at("material_specular") = vec3(0.0f);
        output_colors.at("reflection") = vec3(0.0f);
        output_colors.at("refraction") = vec3(0.0f);
        return {0.0f};
    }

    // Calculate Shading
    const vec3 frag_position = ray.origin + t * ray.dir;

    vec3 diffuse = {0.0f, 0.0f, 0.0f};
    vec3 specular = {0.0f, 0.0f, 0.0f};
    vec3 diffuse_sample = {0.0f, 0.0f, 0.0f};

    CalculateDiffuseAndSpecular(diffuse, specular, ray, material, normal, frag_position, uv, diffuse_sample, face);

    // Reflection and Refraction
    vec3 color_reflected = vec3(0.0f);
    vec3 color_refracted = vec3(0.0f);

    std::stack<std::shared_ptr<Material>> material_stack = {};
    // material stack is a stack of materials of the current medium the ray is in
    // for proper behaviour, DO NOT PARTIALLY OVERLAP meshes with non-zero transmittance!

    if (ctx->max_recursion > 0) {
        if (ctx->reflection && material->specular != vec3(0.0f)) {
            Ray reflected_ray = Ray(frag_position, reflect(ray.dir, normal).normalize());
            color_reflected = TraceSecondaryRay(reflected_ray, 1, material_stack) * material->specular;
        }

        if (ctx->refraction && material->transmittance != vec3(0.0)) {
            vec3 refracted_dir = refract(-ray.dir, normal, 1, material->ior).normalize();
            const float dot_product = dot(refracted_dir, normal);
            if (dot_product < 0) { // refracted ray is actually refracted, otherwise ignore
                material_stack.emplace(material);
                color_refracted = TraceSecondaryRay(Ray(frag_position, refracted_dir), 1, material_stack) * material->transmittance;
            }
        }
    }

    diffuse.vec_clamp(0.0f, 1.0f);
    specular.vec_clamp(0.0f, 1.0f);
    color_reflected.vec_clamp(0.0f, 1.0f);
    color_refracted.vec_clamp(0.0f, 1.0f);

    // Writing to Buffers and Return
    const vec3 color_out = (diffuse + specular + material->emissive + color_reflected + color_refracted);

    ctx->passes.at("depth").data[(y * ctx->width + x)] = depth;
    output_colors.at("combined") = color_out * 255.0f;
    output_colors.at("diffuse") = diffuse * 255.0f;
    output_colors.at("specular") = specular * 255.0f;
    output_colors.at("emissive") = material->emissive * 255.0f;
    output_colors.at("material_diffuse") = diffuse_sample * 255.0f;
    output_colors.at("material_specular") = material->specular * 255.0f;
    output_colors.at("reflection") = color_reflected * 255.0f;
    output_colors.at("refraction") = color_refracted * 255.0f;

    return color_out;
}
