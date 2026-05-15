/**
 * @file gpu_render_engine.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_RenderEngine class
 */

#pragma once
#include "gpu_context.cuh"
#include "gpu_scene.cuh"
#include "gpu_rand.cuh"
#include "gpu_pair.cuh"

/**
 * @class gpu_RenderEngine
 *
 * GPU compatible RenderEngine class
 */
class gpu_RenderEngine {
public:
    gpu_Scene* scene;       /**< Scene to be rendered */
    gpu_Context* ctx;       /**< Rendering and App context */

    gpu_Pair<int>* expensive_rays_coordinates;      /**< Array of Expensive for GPU optimization */
    // top indicates next index to write to
    int expensive_rays_coordinates_top = 0;         /**< Next free element in the array */

    int state_stack_panic = false;                  /**< Panic toggle if recursion depth exceeds hardware limit */

    unsigned long long int max_recursion_depth_sum = 0;     /**< Counter for maximum depth reached in rendering */

    /**
     * Basic class constructor, assigns params and allocates max possible memory for expensive ray optimization
     *
     * @param[in] scene         scene to render
     * @param[in] ctx           render context
     */
    __host__
    gpu_RenderEngine(gpu_Scene* scene, gpu_Context* ctx): scene(scene), ctx(ctx) {
        cudaMallocManaged(&expensive_rays_coordinates, ctx->width * ctx->height * sizeof(gpu_Pair<int>));
        cudaMemset(&expensive_rays_coordinates, 0, ctx->width * ctx->height * sizeof(gpu_Pair<int>));
    }

    /**
     * Basic class destructor
     */
    __host__
    ~gpu_RenderEngine() {
        if (expensive_rays_coordinates != nullptr) cudaFree(expensive_rays_coordinates);
    }

    /**
     * Renders the scene
     * Each thread calculates its x and y coordinate and renders it
     * Also handles progress printing
     *
     * @param[in] thread_counter    counter of finished threads, used for progress printing
     */
    __device__
    void Render(int* thread_counter);

    /**
     * Renders the scene's expensive rays
     * Each thread takes an x and y coordinate from the array and renders it
     * Also handles progress printing
     *
     * @param[in] thread_counter    counter of finished threads, used for progress printing
     */
    __device__
    void RenderExpensive(int* thread_counter);
private:
    /**
     * Marks the current x and y coordinate as expensive (appends it to the array)
     */
    __device__
    void MarkAsExpensive();

    /**
     * Renders the scene at the x and y coordinate
     *
     * @param[in] x     x coordinate to render
     * @param[in] y     y coordinate to render
     */
    __device__
    void RenderXYPixel(int x, int y);

    /**
     * Calculates whether the points from and to are visible to each other
     * This version simply iterates over the entire scene until it hits an objects that blocks visibility
     *
     * @param[in] from      one point to test
     * @param[in] to        other point to test
     */
    __device__
    bool VisibilityToPoint(gpu_vec3 from, gpu_vec3 to) const;

    /**
     * Calculates whether the points from and to are visible to each other
     * This version uses BVH to navigate the scene efficiently until it hits an objects that blocks visibility
     *
     * @param[in] from      one point to test
     * @param[in] to        other point to test
     */
    __device__
    bool VisibilityToPointBVH(gpu_vec3 from, gpu_vec3 to) const;

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
    __device__
    void RayIntersection(const gpu_Ray& ray, gpu_Material** material, gpu_vec3& normal, gpu_vec2& uv, float& depth, float& t, gpu_triangle& out_face, bool culling = true, float epsilon = bigger_epsilon) const;

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
    __device__
    void RayIntersectionBVH(const gpu_Ray& ray, gpu_Material** material, gpu_vec3& normal, gpu_vec2& uv, float& depth, float& t, gpu_triangle& out_face, bool culling = true, float epsilon = bigger_epsilon) const;

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
     * @param[in] rand              random number generator
     * @param[in] face              fragment's face
     */
    __device__
    void CalculateDiffuseAndSpecular(gpu_vec3& diffuse, gpu_vec3& specular, const gpu_Ray& ray, const gpu_Material* material, gpu_vec3& normal, const gpu_vec3& frag_position, const gpu_vec2& uv, gpu_vec3& diffuse_sample, gpu_Rand& rand, const gpu_triangle& face) const;

    /**
     * Traces the primary ray
     * Finds the intersection of the ray, calculates the lighting and writes it to the output
     *
     * @param[in] ray               the primary ray
     * @param[out] output_colors    array of the output color passes
     * @param[in] rand              random number generator
     */
    __device__
    gpu_vec3 TracePrimaryRay(const gpu_Ray& ray, gpu_vec3* output_colors, gpu_Rand& rand);

    /**
     * Traces a secondary ray
     * Finds the intersection of the ray, calculates the lighting and returns its color
     * There is no actual recursion, the recursive algorithm is faked using a while loop
     *
     * @param[in] ray                   the secondary ray
     * @param[in] recursion_depth       depth of the ray tracing recursion
     * @param[in] material_stack        stack of materials previously encountered, used for refraction calculation
     * @param[in] material_stack_top    top of the material stack
     * @param[in] rand                  random number generator
     */
    __device__
    gpu_vec3 TraceSecondaryRay(const gpu_Ray& ray, int recursion_depth, gpu_Material** material_stack, int material_stack_top, gpu_Rand& rand);

    /**
     * @struct TraceState
     *
     * Used in the fake recursion of TraceSecondaryRay to simulate the call stack of a CPU recursion
     */
    struct TraceState {
        int stage;                      /**< Current stage of the fake recursion */
        gpu_Ray ray;                    /**< Current ray */
        int recursion_depth;            /**< Current recursion depth */
        gpu_vec3 normal;                /**< Current normal vector */
        gpu_Material* material;         /**< Current material */
        float incoming_dot_product;     /**< Current incoming dot product */
        gpu_vec3 frag_position;         /**< Current fragment position */

        gpu_vec3 diffuse;               /**< Current diffuse component */
        gpu_vec3 specular;              /**< Current specular component */
        gpu_vec3 reflected;             /**< Current reflected component */
        gpu_vec3 refracted;             /**< Current refracted component */

        bool has_reflected = false;     /**< Whether the state has a reflection component */
        bool has_refracted = false;     /**< Whether the state has a refraction component */

        int material_stack_top;         /**< Current material stack top */
        gpu_Material* material_stack[gpu_Context::gpu_max_recursion];   /**< Current material stack */

        /**
         * A constructor for a new state, that has just entered the first stage
         * Material stack is copied element by element, since it is easier than cudaMemcpy
         *
         * @param ray                       the secondary ray
         * @param recursion_depth           depth of the ray tracing recursion
         * @param material_stack_top        top of the material stack
         * @param material_stack_to_copy    stack of materials previously encountered, used for refraction calculation
         */
        __device__
        TraceState(const gpu_Ray &ray, const int recursion_depth, const int material_stack_top, gpu_Material** material_stack_to_copy): stage(0), ray(ray), recursion_depth(recursion_depth), normal({0.0}), material(nullptr), \
                                                                                                                                        incoming_dot_product(0), frag_position({0.0}), diffuse({0.0}), specular({0.0}), reflected({0.0}), refracted({0.0}), material_stack_top(material_stack_top) {
            for (size_t i = 0; i < gpu_Context::gpu_max_recursion; i++) {
                material_stack[i] = material_stack_to_copy[i];
            }
        }

        /**
         * Empty constructor for static array zero initialization
         */
        __device__
        TraceState(): stage(0), ray({0.0}, {0.0}), recursion_depth(0), normal({0.0}), material(nullptr), incoming_dot_product(0), frag_position({0.0}), diffuse({0.0}), specular({0.0}), reflected({0.0}), refracted({0.0}), material_stack_top(0){}
    };
};
