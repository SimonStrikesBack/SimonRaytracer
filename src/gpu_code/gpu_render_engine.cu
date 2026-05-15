/**
 * @file gpu_render_engine.cu
 * @author Simon Tanev
 * @brief Implementation file for gpu_render_engine.cuh, see the header file for descriptions
 */

#include "gpu_render_engine.cuh"

#include "gpu_mat3.cuh"
#include "gpu_geomath.cuh"

__device__
void gpu_RenderEngine::Render(int* thread_counter) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= ctx->width || y >= ctx->height) return;;

    RenderXYPixel(x, y);

    const int render_size = ctx->width * ctx->height;
    const int step = gpu_max(1, render_size / 100);

    const int old_val = atomicAdd(thread_counter, 1);
    const int new_val = old_val + 1;

    if ((old_val / step) != (new_val / step)) {
        printf("\033[F\033[KProgress: %d%%\n", static_cast<int>(static_cast<float>(new_val) / static_cast<float>(render_size) * 100));
    }
}

__device__
void gpu_RenderEngine::RenderExpensive(int* thread_counter) {
    const int globalIdx = blockIdx.x * blockDim.x + threadIdx.x;

    if (globalIdx >= expensive_rays_coordinates_top) return;

    int x = expensive_rays_coordinates[globalIdx].first;
    int y = expensive_rays_coordinates[globalIdx].second;

    if (x >= ctx->width || y >= ctx->height) return;


    RenderXYPixel(x , y);

    const int jobs = expensive_rays_coordinates_top;
    const int step = gpu_max(1, jobs / 100);

    const int old_val = atomicAdd(thread_counter, 1);
    const int new_val = old_val + 1;

    if ((old_val / step) != (new_val / step)) {
        printf("\033[F\033[KProgress: %d%%\n", static_cast<int>(static_cast<float>(new_val) / static_cast<float>(jobs) * 100));
    }
}

__device__
void gpu_RenderEngine::MarkAsExpensive() {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    const int index = atomicAdd(&expensive_rays_coordinates_top, 1);
    expensive_rays_coordinates[index].first = x;
    expensive_rays_coordinates[index].second = y;
}

__device__
void gpu_RenderEngine::RenderXYPixel(const int x, const int y) {
    gpu_vec3 p_xy = ctx->p_00 + static_cast<float>(x) * ctx->q_w - static_cast<float>(y) * ctx->q_h;
    gpu_Ray ray(ctx->origin, p_xy.normalize());

    gpu_vec3 output_colors[9] = {};
    auto rand = gpu_Rand(x*y);

    TracePrimaryRay(ray, output_colors, rand);

    for (size_t i = 0; i < 9; i++) {
        ctx->passes[i].Write(x, y, output_colors[i], ctx->width, ctx->height);
    }

}

__device__
bool gpu_RenderEngine::VisibilityToPoint(const gpu_vec3 from, const gpu_vec3 to) const {
    const gpu_Ray ray = gpu_Ray(from, (to - from).normalize());
    float depth = INFINITY;
    const float ray_length = (to - from).length();

    for (size_t i = 0; i < scene->geometry_size; i++) {
        float b1, b2;
        const float t = scene->geometry[i].intersect(ray, true, b1, b2);
        if(t == INFINITY) continue;
        if (t < bigger_epsilon) continue;

        const float current_depth = (t * ray.dir).length();
        if(not(current_depth < depth)) continue;
        if (ray_length > current_depth + bigger_epsilon) return false;

        depth = current_depth;

    }

    return (ray_length <= depth + bigger_epsilon);
}

__device__
bool gpu_RenderEngine::VisibilityToPointBVH(const gpu_vec3 from, const gpu_vec3 to) const {
    constexpr size_t stack_size = 64;

    size_t indices[stack_size];
    long int top = 0; // top points to the top valid idx, next free space is at top + 1

    indices[top] = 0;

    const gpu_Ray ray = gpu_Ray(from, (to - from).normalize());
    float depth = INFINITY;
    float t = INFINITY;
    const float ray_length = (to - from).length();

    while (top >= 0) {
        const size_t index = indices[top];
        top--;

        const auto& node = scene->compact_BVH[index];
        const float box_hit_t = node.box.RayIntersection(ray);

        if (box_hit_t < t) {
            if (node.box.has_face == false) { // not a leaf node
                if (node.right_idx == SIZE_MAX) {top++; indices[top] = node.left_idx;}          // if not a leaf, then at least one child must exist
                else if (node.left_idx == SIZE_MAX) {top++; indices[top] = node.right_idx;}     // if not a leaf, then at least one child must exist
                else { // decide on order if both children exist
                    const float left_length = (ray.origin - scene->compact_BVH[node.left_idx].box.centroid).length();
                    const float right_length = (ray.origin - scene->compact_BVH[node.right_idx].box.centroid).length();

                    const size_t first_index = left_length < right_length ? node.left_idx : node.right_idx;
                    const size_t second_index = left_length < right_length ? node.right_idx : node.left_idx;

                    // pushing to stack, so second index is to be pushed first
                    // this grows the stack, so bound checking is needed
                    // if top is at the last possible element, only check the nearer child (not exact, but better than segfault)
                    top++;
                    if (top == (stack_size - 1)){indices[top] = first_index;}
                    else {
                        indices[top] = second_index;
                        top++;
                        indices[top] = first_index;
                    }
                }
            }
            else { // is a leaf node
                const auto& face = node.box.face;

                float b1, b2;
                const float current_t = face.intersect(ray, true, b1, b2);
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

__device__
void gpu_RenderEngine::RayIntersection(const gpu_Ray &ray, gpu_Material** material, gpu_vec3& normal, gpu_vec2& uv, float& depth, float& t, gpu_triangle& out_face, const bool culling, const float epsilon) const {
    for (size_t i = 0; i < scene->geometry_size; i++) {
        auto face = scene->geometry[i];
        float b1, b2;
        const float current_t = face.intersect(ray, culling, b1, b2, epsilon);
        if (current_t < bigger_epsilon) continue;
        if(current_t == INFINITY) continue;

        const float current_depth = (current_t * ray.dir).length();
        if(not(current_depth < depth)) continue;

        *material = face.material;
        depth = current_depth;
        t = current_t;

        const float b3 = 1 - (b1 + b2);
        normal = (face.a.normal * b3 + face.b.normal * b1 + face.c.normal * b2).normalize();
        uv = (face.a.uv * b3 + face.b.uv * b1 + face.c.uv * b2);
        out_face = face;

    }
}

__device__
void gpu_RenderEngine::RayIntersectionBVH(const gpu_Ray &ray, gpu_Material** material, gpu_vec3& normal, gpu_vec2& uv, float& depth, float& t, gpu_triangle& out_face, const bool culling, const float epsilon) const{
    constexpr size_t stack_size = 64;

    size_t indices[stack_size];
    long int top = 0; // top points to the top valid idx, next free space is at top + 1

    indices[top] = 0;

    while (top >= 0) {
        const size_t index = indices[top];
        top--;

        const auto& node = scene->compact_BVH[index];
        const float box_hit_t = node.box.RayIntersection(ray);

        if (box_hit_t < t) {
            if (node.box.has_face == false) { // not a leaf node
                if (node.right_idx == SIZE_MAX) {top++; indices[top] = node.left_idx;}         // if not a leaf, then at least one child must exist
                else if (node.left_idx == SIZE_MAX) {top++; indices[top] = node.right_idx;}    // if not a leaf, then at least one child must exist
                else { // decide on order if both children exist
                    const float left_length = (ray.origin - scene->compact_BVH[node.left_idx].box.centroid).length();
                    const float right_length = (ray.origin - scene->compact_BVH[node.right_idx].box.centroid).length();

                    const size_t first_index = left_length < right_length ? node.left_idx : node.right_idx;
                    const size_t second_index = left_length < right_length ? node.right_idx : node.left_idx;

                    // pushing to stack, so second index is to be pushed first
                    // this grows the stack, so bound checking is needed
                    // if top is at the last possible element, only check the nearer child (not exact, but better than segfault)
                    top++;
                    if (top == (stack_size - 1)){indices[top] = first_index;}
                    else {
                        indices[top] = second_index;
                        top++;
                        indices[top] = first_index;
                    }
                }
            }
            else { // is a leaf node
                const auto& face = node.box.face;

                float b1, b2;
                const float current_t = face.intersect(ray, culling, b1, b2, epsilon);
                if (current_t < bigger_epsilon) continue;
                if(current_t == INFINITY) continue;

                const float current_depth = (current_t * ray.dir).length();
                if(not(current_depth < depth)) continue;

                *material = face.material;
                depth = current_depth;
                t = current_t;

                const float b3 = 1 - (b1 + b2);
                normal = (face.a.normal * b3 + face.b.normal * b1 + face.c.normal * b2).normalize();
                uv = (face.a.uv * b3 + face.b.uv * b1 + face.c.uv * b2);
                out_face = face;
            }
        }
    }
}

__device__
void gpu_RenderEngine::CalculateDiffuseAndSpecular(gpu_vec3& diffuse, gpu_vec3& specular, const gpu_Ray& ray, const gpu_Material* material, gpu_vec3& normal, const gpu_vec3& frag_position, const gpu_vec2& uv, gpu_vec3& diffuse_sample, gpu_Rand& rand, const gpu_triangle& face) const {
    gpu_vec3 base_diffuse = material->diffuse;
    if (material->texture != nullptr) {
        base_diffuse = material->texture->sample_uv(uv);
    }

    diffuse_sample = base_diffuse; // assigning here, since it does not make sense after potential BRDF sampling which is not universal

    // BRDF Code is commented out for the time being
    float view_elevation;
    float view_azimuth;
    float light_elevation;
    float light_azimuth;
    gpu_mat3 world_to_local;

    if (material->normal_map != nullptr && ctx->normal_mapping) {
        gpu_vec3 bitangent;
        gpu_vec3 tangent;
        uvGradientsONB(normal, tangent, bitangent, face);
        const gpu_mat3 normal_to_world = gpu_mat3(tangent, bitangent, normal);
        gpu_vec3 tex_normal = material->normal_map->sample_uv_normal(uv);
        normal = (normal_to_world * tex_normal).normalize();
    }

    if (material->brdf != nullptr) {
        gpu_vec3 bitangent;
        gpu_vec3 tangent;
        branchlessONB(normal, tangent, bitangent);
        world_to_local = gpu_mat3(tangent, bitangent, normal);

        localElevationAzimuth(-ray.dir, world_to_local, view_elevation, view_azimuth); // from the sample back to the viewer
    }

    // Per light components
    for (size_t j = 0; j < scene->area_lights_size; j++) {
        auto area = scene->area_lights[j];

        for (int i = 0; i < ctx->area_light_samples; i++) {
            const float r1 = rand.nextFloat();
            const float r2 = rand.nextFloat();
            const float u = 1.0f - sqrt(r1);
            const float v = (1.0f-r2)*sqrt(r1);

            const gpu_vec3 sampled_point = area.face.a.position + u * (area.face.b.position - area.face.a.position) + v * (area.face.c.position - area.face.a.position);
            bool visible = true;
            if (ctx->shadows) visible = ctx->BVH ? VisibilityToPointBVH(frag_position, sampled_point) : VisibilityToPoint(frag_position, sampled_point);
            if (not visible) continue;

            const gpu_vec3 d_l = (frag_position - sampled_point).normalize();
            const float intensity = (area.area * (dot(normal, d_l)) * (dot(area.normal,-d_l))) / (static_cast<float>(ctx->area_light_samples) * pow((sampled_point - frag_position).length(), 2));
            const gpu_vec3 light_color = area.color * intensity;

            const gpu_vec3 light_direction = (sampled_point - frag_position).normalize();
            const gpu_vec3 reflected_light = reflect(-light_direction, normal);

            if (material->brdf != nullptr) {
                localElevationAzimuth(light_direction, world_to_local, light_elevation, light_azimuth);

                double RGB[3];
                material->brdf->lookup_brdf_val(light_elevation, light_azimuth, view_elevation, view_azimuth, RGB);

                base_diffuse = gpu_vec3(RGB[0], RGB[1], RGB[2]);
                diffuse += base_diffuse * light_color;
                specular = gpu_vec3(0.0);
            }

            else {
                diffuse += light_color * base_diffuse * gpu_max(dot(light_direction, normal), 0.0f);
                specular += light_color * material->specular * pow(gpu_max(dot(reflected_light, -ray.dir), 0.0f), material->shininess);
            }
        }

    }
    for (size_t j = 0; j < scene->point_lights_size; j++){
        auto point = scene->point_lights[j];

        bool visible = true;
        if (ctx->shadows) visible = ctx->BVH ? VisibilityToPointBVH(frag_position, point.position) : VisibilityToPoint(frag_position, point.position);
        if (not visible) continue;

        const gpu_vec3 light_direction = (point.position - frag_position).normalize();
        const gpu_vec3 reflected_light = reflect(-light_direction, normal);

        if (material->brdf != nullptr) {
            localElevationAzimuth(light_direction, world_to_local, light_elevation, light_azimuth);

            double RGB[3];
            material->brdf->lookup_brdf_val(light_elevation, light_azimuth, view_elevation, view_azimuth, RGB);

            base_diffuse = gpu_vec3(RGB[0], RGB[1], RGB[2]);
            diffuse += base_diffuse * point.color;
            specular = gpu_vec3(0.0f);
        }
        else {
            diffuse += point.color * base_diffuse * gpu_max(dot(light_direction, normal), 0.0f);
            specular += point.color * material->specular * pow(gpu_max(dot(reflected_light, -ray.dir), 0.0f), material->shininess);
        }
    }
}

__device__
gpu_vec3 gpu_RenderEngine::TraceSecondaryRay(const gpu_Ray& ray, int recursion_depth, gpu_Material** material_stack, int material_stack_top, gpu_Rand& rand) {
    TraceState state_stack[gpu_Context::gpu_max_recursion];
    int state_stack_top = -1; // state_stack_top points to the top valid idx, next free space is at state_stack_top + 1

    gpu_vec3 result(0.0f);

    unsigned long long int max_recursion_depth = 1;

    state_stack_top++;
    //state_stack[state_stack_top] = TraceState(0, ray, recursion_depth, {0.0}, nullptr, 0, {0.0}, {0.0},{0.0},{0.0},{0.0}, material_stack_top);
    state_stack[state_stack_top] = TraceState(ray, recursion_depth, material_stack_top, material_stack);

    while (state_stack_top >= 0) {
        TraceState* state = &state_stack[state_stack_top];
        if (state->recursion_depth > max_recursion_depth) max_recursion_depth = state->recursion_depth;

        // Calculating local color and calling reflection
        if (state->stage == 0) {
            if (ctx->expensive_rays && state->recursion_depth > ctx->expensive_threshold) {
                MarkAsExpensive();
                return {0.0f};
            }

            float depth = INFINITY;
            float t = INFINITY;
            gpu_vec2 uv(0.0);

            bool culling = true;
            if (state->material_stack_top >= 0) culling = false;

            gpu_triangle face = gpu_triangle();

            if (ctx->BVH) RayIntersectionBVH(state->ray, &state->material, state->normal, uv, depth, t,face, culling);
            else RayIntersection(state->ray, &state->material, state->normal, uv, depth, t, face,culling);

            if (depth == INFINITY) {
                result = {0.0};
                state_stack_top--;
                state->stage = 3;
                continue;
            }

            state->incoming_dot_product = dot(state->ray.dir, state->normal);
            if (state->incoming_dot_product > 0) state->normal = -state->normal;

            state->frag_position = state->ray.origin + t * state->ray.dir;

            gpu_vec3 _ = {0.0f, 0.0f, 0.0f};

            CalculateDiffuseAndSpecular(state->diffuse, state->specular, state->ray, state->material, state->normal, state->frag_position, uv, _, rand, face);

            if (state->recursion_depth < ctx->max_recursion) {
                if (ctx->reflection && state->material->specular != gpu_vec3(0.0f)) {
                    gpu_Ray reflected_ray = gpu_Ray(state->frag_position, reflect(state->ray.dir, state->normal));

                    // Reflection call
                    state->has_reflected = true;
                    state_stack_top++;
                    if (state_stack_top >= gpu_Context::gpu_max_recursion) { // PANIC and abort this pixel
                        if (atomicCAS(&state_stack_panic, false, true) == false) {
                            printf("A pixel recursion has exceeded max recursion level: %d\n", gpu_Context::gpu_max_recursion);
                        }
                        return {0.0f};
                    }
                    //state_stack[state_stack_top] = TraceState(0, reflected_ray, state.recursion_depth + 1, {0.0}, nullptr,0, {0.0}, {0.0},{0.0},{0.0},{0.0}, state.material_stack_top);
                    state_stack[state_stack_top] = TraceState(reflected_ray, state->recursion_depth + 1, state->material_stack_top, state->material_stack);
                }
            }

            state->stage = 1;
            continue;
        }
        // Between reflection and refraction call
        if (state->stage == 1) {
            if (state->has_reflected) {
                state->reflected = result * state->material->specular;
            }

            if (state->recursion_depth < ctx->max_recursion) {
                if (ctx->refraction && state->material->transmittance != gpu_vec3(0.0)) {
                    float refraction_first = 1;
                    float ior_second = 1;
                    if (state->material_stack_top < 0) { // air -> material
                        refraction_first = 1;
                        ior_second = state->material->ior;
                    }
                    else if (state->incoming_dot_product < 0) { // material_stack.top -> material, dir and og normal go against each other
                        refraction_first = state->material_stack[state->material_stack_top]->refraction;
                        ior_second = state->material->ior;
                    }
                    else { // material_stack.top -> material_stack.one_after_top, dir and og normal go in the same direction
                        gpu_Material* top_material = state->material_stack[state->material_stack_top];
                        refraction_first = top_material->refraction;
                        state->material_stack_top--;

                        if (state->material_stack_top < 0) ior_second = 1; // material_stack.top -> air
                        else ior_second = state->material_stack[state->material_stack_top]->ior;

                        state->material_stack_top++;
                        state->material_stack[state->material_stack_top] = top_material; // material stack is to be unchanged at this point
                    }

                    gpu_vec3 refracted_dir = refract(-state->ray.dir, state->normal, refraction_first, ior_second);
                    if (refracted_dir != gpu_vec3(0.0f)) {
                        const float outcoming_dot_product = dot(refracted_dir, state->normal);
                        if (outcoming_dot_product < 0) { // refracted ray is actually refracted, otherwise, ignore it
                            if (state->incoming_dot_product < 0) {
                                state->material_stack_top++;
                                state->material_stack[state->material_stack_top] = state->material;
                            }     // The ray is going inside the mesh with material
                            else {
                                if (state->material_stack_top >= 0) {
                                    state->material_stack_top--;
                                } // The ray is leaving the mesh with material at the top of material_stack, or it is leaving to air
                            }
                            gpu_Ray refracted_ray(state->frag_position, refracted_dir);

                            // Refraction call
                            state->has_refracted = true;
                            state_stack_top++;
                            if (state_stack_top >= gpu_Context::gpu_max_recursion) { // PANIC and abort this pixel
                                if (atomicCAS(&state_stack_panic, false, true) == false) {
                                    printf("A pixel recursion has exceeded max recursion level: %d\n", gpu_Context::gpu_max_recursion);
                                }
                                return {0.0f};
                            }
                            //state_stack[state_stack_top] = TraceState(0, refracted_ray, state.recursion_depth + 1, {0.0}, nullptr, 0, {0.0}, {0.0},{0.0},{0.0},{0.0}, state.material_stack_top);
                            state_stack[state_stack_top] = TraceState(refracted_ray, state->recursion_depth + 1, state->material_stack_top, state->material_stack);
                        }
                    }
                }
            }

            state->stage = 2;
            continue;
        }
        // After refraction call
        if (state->stage == 2) {
            if (state->has_refracted) {
                state->refracted = result * state->material->transmittance;
            }

            result = state->diffuse + state->specular + state->material->emissive + state->reflected + state->refracted;
            state_stack_top--;
            state->stage = 3;
            continue;
        }
    }

    atomicAdd(&max_recursion_depth_sum, max_recursion_depth);

    return result;
}


__device__
gpu_vec3 gpu_RenderEngine::TracePrimaryRay(const gpu_Ray& ray, gpu_vec3* output_colors, gpu_Rand& rand) {
    // Find intersection point and extract its properties
    gpu_vec3 normal;
    gpu_vec2 uv;
    gpu_Material* material = nullptr;
    float depth = INFINITY;
    float t = INFINITY;

    gpu_triangle face = gpu_triangle();

    if (ctx->BVH) RayIntersectionBVH(ray, &material, normal, uv, depth, t, face,true, smaller_epsilon);
    else RayIntersection(ray, &material, normal, uv, depth, t,face, true, smaller_epsilon);

    if (depth == INFINITY) {
        output_colors[gpu_Context::e_depth] = gpu_vec3(depth);
        output_colors[gpu_Context::e_combined] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_diffuse] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_specular] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_emissive] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_material_diffuse] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_material_specular] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_reflection] = gpu_vec3(0.0f);
        output_colors[gpu_Context::e_refraction] = gpu_vec3(0.0f);

        return {0.0f};
    }

    // Calculate Shading
    const gpu_vec3 frag_position = ray.origin + t * ray.dir;

    gpu_vec3 diffuse = {0.0f, 0.0f, 0.0f};
    gpu_vec3 specular = {0.0f, 0.0f, 0.0f};
    gpu_vec3 diffuse_sample = {0.0f, 0.0f, 0.0f};

    CalculateDiffuseAndSpecular(diffuse, specular, ray, material, normal, frag_position, uv, diffuse_sample, rand, face);

    // Reflection and Refraction
    auto color_reflected = gpu_vec3(0.0f);
    auto color_refracted = gpu_vec3(0.0f);

    gpu_Material* material_stack[gpu_Context::gpu_max_recursion];

    for (auto & i : material_stack) {
        i = nullptr;
    }

    int stack_top = -1;
    // material stack is a stack of materials of the current medium the ray is in
    // for proper behaviour, DO NOT PARTIALLY OVERLAP meshes with non-zero transmittance!
    // stack_top points to the top valid idx, next free space is at stack_top + 1

    if (ctx->max_recursion > 0) {
        if (ctx->reflection && material->specular != gpu_vec3(0.0f)) {
            gpu_Ray reflected_ray = gpu_Ray(frag_position, reflect(ray.dir, normal).normalize());
            color_reflected = TraceSecondaryRay(reflected_ray, 1, material_stack, stack_top, rand) * material->specular;
        }

        if (ctx->refraction && material->transmittance != gpu_vec3(0.0)) {
            gpu_vec3 refracted_dir = refract(-ray.dir, normal, 1, material->ior).normalize();
            const float dot_product = dot(refracted_dir, normal);
            if (dot_product < 0) { // refracted ray is actually refracted, otherwise ignore
                stack_top++;
                material_stack[stack_top] = material;
                color_refracted = TraceSecondaryRay(gpu_Ray(frag_position, refracted_dir), 1, material_stack, stack_top, rand) * material->transmittance;
            }
        }
    }

    diffuse.clamp(0.0f, 1.0f);
    specular.clamp(0.0f, 1.0f);
    color_reflected.clamp(0.0f, 1.0f);
    color_refracted.clamp(0.0f, 1.0f);

    // Writing to Buffers and Return
    const gpu_vec3 color_out = (diffuse + specular + material->emissive + color_reflected + color_refracted);

    output_colors[gpu_Context::e_depth] = gpu_vec3(depth);
    output_colors[gpu_Context::e_combined] = color_out * 255.0f;
    output_colors[gpu_Context::e_diffuse] = diffuse * 255.0f;
    output_colors[gpu_Context::e_specular] = specular * 255.0f;
    output_colors[gpu_Context::e_emissive] = material->emissive * 255.0f;
    output_colors[gpu_Context::e_material_diffuse] = diffuse_sample * 255.0f;
    output_colors[gpu_Context::e_material_specular] = material->specular * 255.0f;
    output_colors[gpu_Context::e_reflection] = color_reflected * 255.0f;
    output_colors[gpu_Context::e_refraction] = color_refracted * 255.0f;

    return color_out;
}

