/**
 * @file kernel.cu
 * @author Simon Tanev
 * @brief File containing logic around GPU Kernel calls and GPU compatible class conversions
 */

#include <cstdio>
#include <cuda_runtime.h>
#include "gpu_code/gpu_vec2.cuh"
#include "gpu_code/gpu_vec3.cuh"
#include "gpu_code/gpu_scene.cuh"
#include "gpu_code/gpu_context.cuh"
#include "gpu_code/gpu_render_engine.cuh"
#include "context.hpp"
#include "scene.hpp"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * Print function for CUDA errors
 */
static void HandleError( cudaError_t err, const char *file, int line )
{

    if (err != cudaSuccess)
    {
        printf( "%s in %s at line %d\n", cudaGetErrorString( err ), file, line );
        exit( EXIT_FAILURE );
    }
}

/**
 * Kernel global function that calls the GPU renderer's Render function
 *
 * @param[in] u_RenderEngine    GPU render engine
 * @param[in] thread_counter    Thread counter for progress printing
 */
__global__
void Render(gpu_RenderEngine* u_RenderEngine, int* thread_counter) {
    u_RenderEngine->Render(thread_counter);
}

/**
 * Kernel global function that calls the GPU renderer's RenderExpensive function
 *
 * @param[in] u_RenderEngine    GPU render engine
 * @param[in] thread_counter    Thread counter for progress printing
 */
__global__
void RenderExpensive(gpu_RenderEngine* u_RenderEngine, int* thread_counter) {
    u_RenderEngine->RenderExpensive(thread_counter);
}

#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))

/**
 * Initializes GPU compatible classes, handles CUDA memory, calls kernels and extracts results back from the GPU classes
 *
 * @param[in]  scene    Scene to be converted
 * @param[in]  ctx      Context to be converted
 * @param[out] start    Start time of the render
 * @param[out] end      End time of the render
 */
extern "C" void GPUInitAndRender(const Scene* scene, const Context* ctx, std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end){
    //std::cout << "Converting Scene" << std::endl;
    gpu_Scene* u_scene;
    cudaMallocManaged(&u_scene, sizeof(gpu_Scene));
    new (u_scene) gpu_Scene(*scene);

    //std::cout << "Converting Context" << std::endl;
    gpu_Context* u_ctx;
    cudaMallocManaged(&u_ctx, sizeof(gpu_Context));
    new (u_ctx) gpu_Context(*ctx);

    //std::cout << "Init RenderEngine" << std::endl;
    gpu_RenderEngine* u_RenderEngine;
    cudaMallocManaged(&u_RenderEngine, sizeof(gpu_RenderEngine));
    new (u_RenderEngine) gpu_RenderEngine(u_scene, u_ctx);

    //std::cout << "Setting up and calling GPU kernel(s)" << std::endl;
    int _ = 0;
    int max_block_size = 0;
    HANDLE_ERROR(cudaOccupancyMaxPotentialBlockSize(&_, &max_block_size, Render));

    const int side_size = static_cast<int>(sqrt(max_block_size));

    dim3 blockSize(side_size,side_size);
    dim3 gridSize(
    ((ctx->width  + blockSize.x - 1) / blockSize.x),
    ((ctx->height + blockSize.y - 1) / blockSize.y));

    int* thread_counter;
    cudaMalloc(&thread_counter, sizeof(int));
    cudaMemset(thread_counter, 0, sizeof(int));

    std::cout << "Expensive Rays Optimization: " << std::boolalpha << ctx->expensive_rays;
    if (ctx->expensive_rays) {
        std::cout << ", at Threshold Level: " << ctx->expensive_threshold;
    }
    std::cout << "" << std::endl;

    std::cout << "Rendering in Parallel on GPU" << std::endl;
    std::cout << "" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    Render<<<gridSize, blockSize>>>(u_RenderEngine, thread_counter);
    HANDLE_ERROR(cudaDeviceSynchronize());

    if (u_RenderEngine->ctx->expensive_rays && u_RenderEngine->expensive_rays_coordinates_top > 0) {
        cudaMemset(thread_counter, 0, sizeof(int));
        u_RenderEngine->ctx->expensive_rays = false;

        HANDLE_ERROR(cudaOccupancyMaxPotentialBlockSize(&_, &max_block_size, RenderExpensive));

        max_block_size = u_RenderEngine->expensive_rays_coordinates_top < max_block_size ? u_RenderEngine->expensive_rays_coordinates_top : max_block_size;

        const int numBlocks = (u_RenderEngine->expensive_rays_coordinates_top + max_block_size - 1) / max_block_size;

        std::cout << u_RenderEngine->expensive_rays_coordinates_top << " Expensive Rays Encountered, Re-Rendering Them" << std::endl;
        std::cout << "" << std::endl;

        RenderExpensive<<<numBlocks, max_block_size>>>(u_RenderEngine, thread_counter);
        HANDLE_ERROR(cudaDeviceSynchronize());
    }
    else {
        const double avg_recursion_depth = static_cast<double>(u_RenderEngine->max_recursion_depth_sum) / static_cast<double>(ctx->width * ctx->height);
        std::cout << "Average recursion depth: " << avg_recursion_depth << std::endl;
    }

    end = std::chrono::high_resolution_clock::now();

    //std::cout << "Getting Information out of GPU Render Engine" << std::endl;
    for (auto& [pass_name, pass] : ctx->passes) {
        const gpu_Context::gpu_BufferName name = gpu_Context::gpu_Buffer::BufferNameFromString(pass_name.c_str());
        u_ctx->passes[name].CopyToNormalBuffer(pass, ctx->width, ctx->height);
    }

    ctx->NormalizeDepthBuffer();

    //std::cout << "Calling Scene Destructor" << std::endl;
    u_scene->~gpu_Scene();
    //std::cout << "Calling Ctx destructor" << std::endl;
    u_ctx->~gpu_Context();
    //std::cout << "Calling RenderEngine destructor" << std::endl;
    u_RenderEngine->~gpu_RenderEngine();

    //std::cout << "Freeing Scene, Ctx and RenderEngine Memory" << std::endl;
    cudaFree(u_scene);
    cudaFree(u_ctx);
    cudaFree(u_RenderEngine);
}