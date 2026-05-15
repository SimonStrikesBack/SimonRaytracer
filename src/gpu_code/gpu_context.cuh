/**
 * @file gpu_context.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Context class
 */

#pragma once
#include "gpu_vec3.cuh"
#include "../context.hpp"
#include "gpu_geomath.cuh"

/**
 * @class gpu_Context
 *
 * GPU compatible Context class
 */
class gpu_Context {
public:
    int width;              /**< Width of the render */
    int height;             /**< Height of the render */
    gpu_vec3 origin;        /**< Origin position of the camera */
    gpu_vec3 direction;     /**< Camera direction vector */
    gpu_vec3 up;            /**< Camera up vector */
    gpu_vec3 right;         /**< Camera right vector */
    float FOV;              /**< Camera FOV */
    float g_w;              /**< Projection plane width */
    float g_h;              /**< Projection plane height */
    gpu_vec3 q_w;           /**< Projection plane width pixel offset */
    gpu_vec3 q_h;           /**< Projection plane height pixel offset */
    gpu_vec3 p_00;          /**< Vector to projection plane's top left pixel */

    int area_light_samples;     /**< How many samples to sample an area light by */
    int max_recursion;          /**< Maximum ray tracing recursion depth (akin to max bounces) */
    static constexpr int gpu_max_recursion = 64;    /**< GPU hardware max recursion cap */
    bool BVH;                   /**< Toggle BVH to navigate the scene */
    float scene_scale;          /**< Scale of the scene, when loading, used to divide all position vectors by to unify scene scales for light attenuation */
    bool reflection;            /**< Toggle reflection rays */
    bool refraction;            /**< Toggle refraction rays */
    bool shadows;               /**< Toggle test for shadows when sampling lights */
    bool normal_mapping;        /**< Toggle normal mapping */
    bool expensive_rays;        /**< Toggle expensive rays GPU optimization */
    int expensive_threshold;    /**< Recursion depth at which rays become expensive */

    /**
     * Constructor from a CPU Context
     *
     * @param[in] ctx       CPU Context
     */
    __host__
    gpu_Context(const Context& ctx): width(ctx.width), height(ctx.height), origin(ctx.origin), direction(ctx.direction), up(ctx.up), right(ctx.right), \
                                     FOV(ctx.FOV), g_w(ctx.g_w), g_h(ctx.g_h), q_w(ctx.q_w), q_h(ctx.q_h), p_00(ctx.p_00), area_light_samples(ctx.area_light_samples), BVH(ctx.BVH), \
                                     scene_scale(ctx.scene_scale), reflection(ctx.reflection), refraction(ctx.refraction), shadows(ctx.shadows), normal_mapping(ctx.normal_mapping), \
                                     expensive_rays(ctx.expensive_rays), expensive_threshold(ctx.expensive_threshold), passes_size(ctx.passes.size()) {
        passes = nullptr;
        max_recursion = ctx.max_recursion < gpu_max_recursion ? ctx.max_recursion : gpu_max_recursion;

        // Convert Passes
        cudaMallocManaged(&passes, passes_size * sizeof(gpu_Buffer));

        for (const auto& [name, p] : ctx.passes) {
            size_t i = gpu_Buffer::BufferNameFromString(name.c_str());

            new (&passes[i]) gpu_Buffer(p, name.c_str());
            const size_t size = ctx.width * ctx.height * passes[i].channels * sizeof(float);
            if (passes[i].output) cudaMallocManaged(&(passes[i].data), size);

        }

    }

    /**
     * @enum gpu_BufferName
     *
     * It maps different passes' names to an enum value
     * GPU compatible way to change a string to buffer map to a simple array
     */
    enum gpu_BufferName {
        e_combined = 0,
        e_depth = 1,
        e_diffuse = 2,
        e_specular = 3,
        e_emissive = 4,
        e_material_diffuse = 5,
        e_material_specular = 6,
        e_reflection = 7,
        e_refraction = 8,
    };

    /**
     * @enum gpu_Channels
     * Represents the number of channels of a render pass as enum and the actual number
     */
    enum gpu_Channels {
        RGB = 3,
        BW = 1,
    };

    /**
     * @struct gpu_Buffer
     * Represents the buffer of a pass
     */
    struct gpu_Buffer {
    public:
        float* data;                /**< Raw data of the buffer */
        gpu_BufferName name;        /**< Name of the pass as an enum */
        gpu_Channels channels;      /**< Number of pass channels */
        bool output;                /**< Toggle whether pass will be written to output */

        /**
         * Converts a string pass name to an enum pass name
         *
         * @param[in] name      string name to convert
         */
        __host__
        static gpu_BufferName BufferNameFromString(const char *name) {
            if (strcmp(name, "combined") == 0) return e_combined;
            if (strcmp(name, "depth") == 0) return e_depth;
            if (strcmp(name, "diffuse") == 0) return e_diffuse;
            if (strcmp(name, "specular") == 0) return e_specular;
            if (strcmp(name, "emissive") == 0) return e_emissive;
            if (strcmp(name, "material_diffuse") == 0) return e_material_diffuse;
            if (strcmp(name, "material_specular") == 0) return e_material_specular;
            if (strcmp(name, "reflection") == 0) return e_reflection;
            if (strcmp(name, "refraction") == 0) return e_refraction;
            return static_cast<gpu_BufferName>(-1);
        }

        /**
         * Constructor from a CPU Buffer and its string name
         *
         * @param b         CPU buffer
         * @param b_name    buffer name
         */
        __host__
        gpu_Buffer(const Context::Buffer& b, const char* b_name): output(b.output) {
            data = nullptr;
            if (b.channels == Context::Channels::BW) channels = BW;
            else channels = RGB;
            name = BufferNameFromString(b_name);
        }

        /**
         * Copies the data to a CPU buffer
         *
         * @param b         CPU Buffer
         * @param width     Buffer width
         * @param height    Buffer height
         */
        __host__
        void CopyToNormalBuffer(const Context::Buffer& b, const int width, const int height) const {
            if (static_cast<int>(channels) != static_cast<int>(b.channels)) return;
            if (!output) return;

            memcpy(b.data, data, width * height * channels * sizeof(float));
        }

        /**
         * Writes a value to an x and y position in the buffer
         *
         * @param x         x position to write to
         * @param y         y position to write to
         * @param value     value to write
         * @param width     width of the buffer
         * @param height    height of the buffer
         */
        __device__
        void Write(const int x, const int y, const gpu_vec3 value, const int width, const int height) const {
            if (!output) return;
            if (x >= width || y >= height) return;
            if (channels == BW) {
                if (name == e_depth) data[(y * width + x)] = value.x; // depth buffer is normalized later
                else data[(y * width + x)] = gpu_clamp(value.x, 0.0f, 255.0f);
            }
            else {
                data[3 * (y * width + x) + 0] = gpu_clamp(value.x, 0.0f, 255.0f);
                data[3 * (y * width + x) + 1] = gpu_clamp(value.y, 0.0f, 255.0f);
                data[3 * (y * width + x) + 2] = gpu_clamp(value.z, 0.0f, 255.0f);
            }
        }
    };

    gpu_Buffer* passes;     /**< An array of the different passes' buffers */
    size_t passes_size;     /**< Passes array size */

    /**
     * Destructor of the class
     * It calls free functions on the CUDA memory stored
     */
    __host__
    ~gpu_Context() {
        for (size_t i = 0; i < passes_size; i++) {
            if (passes[i].data != nullptr) cudaFree(passes[i].data);
        }
        cudaFree(passes);
    }
};