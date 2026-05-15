/**
 * @file context.hpp
 * @author Simon Tanev
 * @brief Header file containing the Context class
 */


#ifndef RAYTRACER_CONTEXT_HPP
#define RAYTRACER_CONTEXT_HPP
#pragma once
#include "vec3.hpp"
#include <string>
#include <unordered_map>

/**
 * @class Context
 * Context class holds all the Raytracer option settings
 */
class Context{
public:
    int width;                  /**< Width of the render */
    int height;                 /**< Height of the render */
    vec3 origin;                /**< Origin position of the camera */
    vec3 direction;             /**< Camera direction vector */
    vec3 up;                    /**< Camera up vector */
    vec3 right;                 /**< Camera right vector */
    float FOV;                  /**< Camera FOV */
    float g_w;                  /**< Projection plane width */
    float g_h;                  /**< Projection plane height */
    vec3 q_w;                   /**< Projection plane width pixel offset */
    vec3 q_h;                   /**< Projection plane height pixel offset */
    vec3 p_00;                  /**< Vector to projection plane's top left pixel */
    char scene_path[128];       /**< Path to the scene directory */
    char scene_name[128];       /**< Name of the scene to be loaded in the specified path */
    char output_path[128];      /**< Path to the output directory */
    char brdf_path[128];        /**< Path to the BRDF materials directory */

    /**
     * @enum Parallelism
     * Represents the type of parallelism to be used in rendering
     */
    enum Parallelism {
        NONE,                   /**< CPU Sequential Render */
        CPU,                    /**< CPU Parallel Render */
        GPU,                    /**< GPU Parallel Render */
    };

    int area_light_samples;     /**< How many samples to sample an area light by */
    int max_recursion;          /**< Maximum ray tracing recursion depth (akin to max bounces) */
    bool BVH;                   /**< Toggle BVH to navigate the scene */
    Parallelism parallelism;    /**< Type of parallelism to use in rendering */
    float scene_scale;          /**< Scale of the scene, when loading, used to divide all position vectors by to unify scene scales for light attenuation */
    bool reflection;            /**< Toggle reflection rays */
    bool refraction;            /**< Toggle refraction rays */
    bool shadows;               /**< Toggle test for shadows when sampling lights */
    bool normal_mapping;        /**< Toggle normal mapping */
    bool expensive_rays;        /**< Toggle expensive rays GPU optimization */
    int expensive_threshold;    /**< Recursion depth at which rays become expensive */

    /**
     * @enum Channels
     * Represents the number of channels of a render pass as enum and the actual number
     */
    enum Channels {
        RGB = 3,    /**< RGB color, 3 channels */
        BW = 1,     /**< Black & White, 1 channel */
    };

    /**
     * @struct Buffer
     * Represents the buffer of a pass
     */
    struct Buffer {
        float* data;        /**< Raw data of the buffer */
        const char* name;   /**< Name of the pass */
        Channels channels;  /**< Number of pass channels */
        bool output;        /**< Toggle whether pass will be written to output */

        /**
         * Basic constructor that just assigns all the input fields (does not allocate data memory)
         */
        Buffer(const char* output_path, const Channels channels, const bool output): name(output_path), channels(channels), output(output){ data = nullptr; };
    };

    std::unordered_map<std::string, Buffer> passes; /**< A map of the different passes' buffers and their names */

    /**
     * Constructor that creates a new context from the config at the input path
     * It uses the nlohmann::JSON single header library to parse the JSON config
     * It then calculates the other fields
     *
     * @param[in]   config      Path to the config JSON file to retrieve all information from
     */
    explicit Context(const char* config);

    /**
     * Prints the passes and whether they will be written to output
     */
    void PrintPasses() const;

    /**
     * Prints the help string that helps tells the user the app and config file usage
     */
    static void PrintHelp();

    /**
     * Prints information about render settings
     */
    void PrintRenderInfo() const;

    /**
     * Converts the currently saved parallelism enum to string
     */
    std::string ParallelismToString() const;

    /**
     * Converts the parallelism enum to string
     *
     * @param[in]   parallelism     enum to be converted
     */
    static std::string ParallelismToString(Parallelism parallelism);

    /**
     * Normalizes the Depth Buffer values to [0-255] to allow it to be written to output
     */
    void NormalizeDepthBuffer() const;

};



#endif //RAYTRACER_CONTEXT_HPP
