/**
 * @file gpu_pair.cuh
 * @author Simon Tanev
 * @brief Header file containing the gpu_Pair class
 */

#pragma once

/**
 * @class gpu_Pair
 *
 * GPU compatible templated Pair class
 */
template<typename T>
class gpu_Pair {
public:
    T first;    /**< first element */
    T second;   /**< second element */

    /**
     * Basic class constructor
     *
     * @param[in] first     first element
     * @param[in] second    second element
     */
    gpu_Pair(const T& first,const T& second): first(first), second(second){}
};