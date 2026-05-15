/**
 * @file gpu_rand.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Rand class
 */

#pragma once
#include <curand_kernel.h>

/**
 * @class gpu_Rand
 *
 * GPU compatible Random engine class
 */
class gpu_Rand {
private:
    curandStatePhilox4_32_10_t state;   /**< Internal state engine */
    float4 cache;                       /**< Four value cache */
    int index;                          /**< Index in the cache */

public:
    /**
     * Constructor that initializes the random engine and fills the cache for the first time
     *
     * @param[in] sequence      another value for seeding
     */
    __device__
    gpu_Rand(unsigned long long sequence) : index(0) {
        curand_init(0, sequence, 0, &state);
        cache = curand_uniform4(&state);
    }

    /**
     * Returns a new random float
     * It tries to return a value from the cache
     * If the cache has been used, generates new values to cache
     */
    __device__
    float nextFloat() {
        if (index >= 4) {
            cache = curand_uniform4(&state);
            index = 0;
        }
        const float ret = reinterpret_cast<float *>(&cache)[index];
        index++;
        return ret;
    }
};