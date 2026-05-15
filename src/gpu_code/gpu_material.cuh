/**
 * @file gpu_material.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Material class
 */

#pragma once
#include "gpu_BRDF.cuh"
#include "gpu_vec3.cuh"
#include "gpu_texture.cuh"
#include "../material.hpp"

/**
 * @class gpu_Material
 *
 * GPU compatible Material class
 */
class gpu_Material {
public:
    gpu_vec3 ambient;           /**< Ambient color component */
    gpu_vec3 diffuse;           /**< Diffuse color component */
    gpu_vec3 specular;          /**< Specular color component */
    gpu_vec3 emissive;          /**< Emissive color component */
    float shininess;            /**< Shininess exponent */

    //Advanced
    gpu_vec3 transmittance;     /**< Material transmittance */
    float ior;                  /**< Material ior (ior = 1 / refraction index) */
    float refraction;           /**< Material refraction index (refraction index = 1 / ior) */
    float dissolve;             /**< Material dissolve */

    gpu_BRDF* brdf;             /**< Pointer to BRDF material override */
    gpu_Texture* texture;       /**< Pointer to diffuse texture */
    gpu_Texture* normal_map;    /**< Pointer to normal texture */

    /**
     * Basic Constructor that just assigns all the input fields, calculates the ior inverse, and sets pointers to null
     *
     * Textures and BRDF are assigned later manually
     *
     * @param[in] diffuse                   diffuse component
     * @param[in] specular                  specular component
     * @param[in] emissive                  emissive component
     * @param[in] shininess                 shininess value
     * @param[in] transmittance             transmittance value
     * @param[in] ior                       ior value
     * @param[in] dissolve                  dissolve value
     */
    __host__ __device__
    gpu_Material(const gpu_vec3& diffuse, const gpu_vec3& specular, const gpu_vec3& emissive, const float shininess, const gpu_vec3 transmittance, const float ior, const float dissolve) : \
    diffuse(diffuse), specular(specular), emissive(emissive), shininess(shininess), transmittance(transmittance), ior(1.0f / ior), dissolve(dissolve) {
        refraction = ior;
        brdf = nullptr;
        texture = nullptr;
        normal_map = nullptr;
    }

    /**
     * Constructor from a CPU Material
     *
     * Textures and BRDF are assigned later manually
     *
     * @param[in] m     CPU Material
     */
    __host__
    gpu_Material(const Material& m): ambient(m.ambient), diffuse(m.diffuse), specular(m.specular), emissive(m.emissive), shininess(m.shininess), transmittance(m.transmittance), ior(m.ior), \
    refraction(m.refraction), dissolve(m.dissolve){
        brdf = nullptr;
        texture = nullptr;
        normal_map = nullptr;
    }

};


