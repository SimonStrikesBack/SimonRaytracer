/**
 * @file material.hpp
 * @author Simon Tanev
 * @brief Header file containing the Material class
 */

#ifndef RAYTRACER_MATERIAL_HPP
#define RAYTRACER_MATERIAL_HPP
#pragma once
#include "vec3.hpp"
#include <string>
#include <utility>
#include <memory>
#include "texture.hpp"
#include "include/TBRDF.h"

/**
 * @class Material
 *
 * Represents an object's material
 */
class Material {
public:
    std::string name;       /**< Material name */
    vec3 ambient;           /**< Ambient color component */
    vec3 diffuse;           /**< Diffuse color component */
    vec3 specular;          /**< Specular color component */
    vec3 emissive;          /**< Emissive color component */
    float shininess;        /**< Shininess exponent */

    //Advanced
    vec3 transmittance;     /**< Material transmittance */
    float ior;              /**< Material ior (ior = 1 / refraction index) */
    float refraction;       /**< Material refraction index (refraction index = 1 / ior) */
    float dissolve;         /**< Material dissolve */

    std::string diffuse_texture_name;       /**< Diffuse texture name */
    std::string normal_texture_name;        /**< Normal texture name */
    std::string brdf_name;                  /**< BRDF material name */
    std::shared_ptr<BRDF> brdf;             /**< Pointer to BRDF material override */
    std::shared_ptr<Texture> texture;       /**< Pointer to diffuse texture */
    std::shared_ptr<Texture> normal_map;    /**< Pointer to normal texture */


    /**
     * Basic Constructor that just assigns all the input fields, calculates the ior inverse, and sets pointers to null
     *
     * Textures and BRDF are assigned later manually from the scene init
     *
     * @param[in] diffuse                   diffuse component
     * @param[in] specular                  specular component
     * @param[in] emissive                  emissive component
     * @param[in] shininess                 shininess value
     * @param[in] transmittance             transmittance value
     * @param[in] ior                       ior value
     * @param[in] dissolve                  dissolve value
     * @param[in] name                      material name
     * @param[in] diffuse_texture_name      diffuse texture name
     * @param[in] normal_texture_name       normal texture name
     */
    Material(const vec3& diffuse, const vec3& specular, const vec3& emissive, const float shininess, const vec3 transmittance, const float ior, const float dissolve, std::string name, std::string diffuse_texture_name, std::string normal_texture_name) : \
    name(std::move(name)), diffuse(diffuse), specular(specular), emissive(emissive), shininess(shininess), transmittance(transmittance), ior(1.0f / ior), dissolve(dissolve), diffuse_texture_name(std::move(diffuse_texture_name)), normal_texture_name(std::move(normal_texture_name)) {
        brdf_name = "";
        refraction = ior;
        brdf = nullptr;
        texture = nullptr;
        normal_map = nullptr;
    }
};


#endif //RAYTRACER_MATERIAL_HPP
