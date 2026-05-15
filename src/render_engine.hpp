/**
 * @file render_engine.hpp
 * @author Simon Tanev
 * @brief File containing RenderEngine class
 */

#ifndef RAYTRACER_RENDER_ENGINE_HPP
#define RAYTRACER_RENDER_ENGINE_HPP
#pragma once
#include <memory>
#include "context.hpp"
#include "scene.hpp"
#include "vec2.hpp"
#include <random>
#include <stack>
#include <queue>
#include <shared_mutex>
#include <latch>

/**
 * @class RenderEngine
 *
 * Class responsible for rendering of the scene (CPU only)
 */
class RenderEngine {
public:
    std::shared_ptr<Context> ctx;       /**< Rendering and App context */
    std::shared_ptr<Scene> scene;       /**< Scene to be rendered */

    thread_local static std::random_device r;                                   /**< Random device for random number generation */
    thread_local static std::minstd_rand engine;                                /**< Engine for random number generation */
    thread_local static std::uniform_real_distribution<float> distribution;     /**< Distribution for random number generation */
    mutable std::shared_mutex mtx;      /**< Mutex for job queue access in parallel rendering */
    mutable std::mutex stdout_mtx;      /**< Mutex for stdout access in parallel rendering */
    mutable size_t finished_threads = 0;

    /**
     * Basic Constructor that just assigns all the input fields and inits the prng static fields
     *
     * @param[in] ctx       rendering context
     * @param[in] scene     scene to be rendered
     */
    RenderEngine(const std::shared_ptr<Context> &ctx, const std::shared_ptr<Scene> &scene): ctx(ctx), scene(scene) {
        engine = std::minstd_rand(r());
        distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
    }

    /**
     * Renders the scene sequentially
     * It is basically just a wrapper around RenderXRange with progress printing
     *
     * @param[out] start     Start time of the render
     * @param[out] end       End time of the render
     */
    void RenderSequential(std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end) const;

    /**
     * Renders the scene in parallel
     * It creates a job queue and a thread pool that renders the scene from the jobs
     * It handles the thread creation and joining
     *
     * @param[out] start     Start time of the render
     * @param[out] end       End time of the render
     */
    void RenderParallelCPU(std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end) const;
    //void RenderParallelGPU() const;

private:
    /**
     * Method for worker threads in parallel rendering
     * It takes a job from the job queue and renders it until it is empty, prints the progress
     *
     * @param[in] job_queue             job queue to take jobs from
     * @param[in] read_queue_latch      latch to synchronize the start of the work of all threads
     */
    void ThreadWorker(std::queue<std::pair<int, int>>& job_queue, std::latch& read_queue_latch) const;

    /**
     * Renders pixels in a range of columns from and to an x coordinate
     * It calls TracePrimaryRay in a loop over the given x range and the entire y range
     *
     * @param[in] from      start of the x range to render
     * @param[in] to        end of the x range to render
     */
    void RenderXRange(int from, int to) const;

    /**
     * Calculates whether the points from and to are visible to each other
     * This version simply iterates over the entire scene until it hits an objects that blocks visibility
     *
     * @param[in] from      one point to test
     * @param[in] to        other point to test
     */
    bool VisibilityToPoint(vec3 from, vec3 to) const;

    /**
     * Calculates whether the points from and to are visible to each other
     * This version uses BVH to navigate the scene efficiently until it hits an objects that blocks visibility
     *
     * @param[in] from      one point to test
     * @param[in] to        other point to test
     */
    bool VisibilityToPointBVH(vec3 from, vec3 to) const;

    /**
     * Calculates the nearest intersection of the ray with scene geometry
     * This version simply iterates over the entire scene to check intersections
     *
     * @param[in] ray           ray to test against geometry
     * @param[out] material     material of intersected face
     * @param[out] normal       interpolated normal of the intersected face
     * @param[out] uv           interpolated uv of the intersected face
     * @param[out] depth        distance from the ray origin to the intersection point
     * @param[out] t            scale factor of the ray direction to the intersection point
     * @param[out] out_face     intersected face
     * @param[in] culling       backface culling toggle
     * @param[in] epsilon       epsilon for floating point shenanigans
     */
    void RayIntersection(const Ray& ray, std::shared_ptr<Material>& material, vec3& normal, vec2& uv, float& depth, float& t, triangle& out_face, bool culling = true, float epsilon = bigger_epsilon) const;

    /**
     * Calculates the nearest intersection of the ray with scene geometry
     * This version uses BVH to navigate the scene efficiently to check intersections
     *
     * @param[in] ray           ray to test against geometry
     * @param[out] material     material of intersected face
     * @param[out] normal       interpolated normal of the intersected face
     * @param[out] uv           interpolated uv of the intersected face
     * @param[out] depth        distance from the ray origin to the intersection point
     * @param[out] t            scale factor of the ray direction to the intersection point
     * @param[out] out_face     intersected face
     * @param[in] culling       backface culling toggle
     * @param[in] epsilon       epsilon for floating point shenanigans
     */
    void RayIntersectionBVH(const Ray& ray, std::shared_ptr<Material>& material, vec3& normal, vec2& uv, float& depth, float& t, triangle& out_face, bool culling = true, float epsilon = bigger_epsilon) const;

    /**
     * Calculates the Diffuse and Specular parts of the rendering equation
     * It samples a BRDF if present, uses the Phong lighting model otherwise
     *
     * @param[out] diffuse          calculated diffuse component
     * @param[out] specular         calculated specular component
     * @param[in] ray               view vector ray
     * @param[in] material          fragment's material
     * @param[in, out] normal       interpolated fragment's normal vector, may get changed if the material uses a normal map to bend the normal
     * @param[in] frag_position     world coordinate position of the fragment
     * @param[in] uv                interpolated fragment's uv coordinates
     * @param[out] diffuse_sample   raw sample of the material diffuse component
     * @param[in] face              fragment's face
     */
    void CalculateDiffuseAndSpecular(vec3& diffuse, vec3& specular, const Ray& ray, const std::shared_ptr<Material> &material, vec3& normal, const vec3& frag_position, const vec2& uv, vec3& diffuse_sample, const triangle& face) const;

    /**
     * Traces the primary ray
     * Finds the intersection of the ray, calculates the lighting and writes it to the output
     *
     * @param[in] ray               the primary ray
     * @param[in] x                 x position in the output
     * @param[in] y                 y position in the output
     * @param[out] output_colors    map of the output color passes
     */
    vec3 TracePrimaryRay(const Ray& ray, int x, int y, \
                         std::unordered_map<std::string, vec3>& output_colors) const;

    /**
     * Traces a secondary ray
     * Finds the intersection of the ray, calculates the lighting and returns its color
     *
     * @param[in] ray               the secondary ray
     * @param[in] recursion_depth   depth of the ray tracing recursion
     * @param[in] material_stack    stack of materials previously encountered, used for refraction calculation
     */
    vec3 TraceSecondaryRay(const Ray& ray, int recursion_depth, std::stack<std::shared_ptr<Material>> material_stack) const;
};


#endif //RAYTRACER_RENDER_ENGINE_HPP
