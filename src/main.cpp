/**
 * @file main.cpp
 * @author Simon Tanev
 * @brief Main entry point of the entire application
 */
#include <iostream>
#include "context.hpp"
#include "scene.hpp"
#include "render_engine.hpp"
#include "OutputHandler.hpp"
#include <chrono>

extern "C" void GPUInitAndRender(const Scene* scene, const Context* ctx, std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::chrono::time_point<std::chrono::high_resolution_clock>& end);

/**
 * Main function
 *
 * It expects a single command line argument, prints the help string otherwise
 * Possible args:
 *      --help - prints the help string and ends
 *      <path_to_config> - tries to load the config of a scene, and renders it
 */
int main(const int argc, char const* argv[]) {
    char config_name[128];
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            Context::PrintHelp();
            return EXIT_SUCCESS;
        }
        strcpy(config_name, argv[1]);
    }
    else {
        Context::PrintHelp();
        return EXIT_SUCCESS;
    }
    std::cout << argv[0] <<  ", " << config_name << std::endl;
    //Init context, scene & renderer
    std::cout << "Init Context" << std::endl;
    const std::shared_ptr<Context> ctx = std::make_shared<Context>(config_name);
    std::cout << "Init Scene" << std::endl;
    const std::shared_ptr<Scene> scene = std::make_shared<Scene>(config_name, ctx->scene_path, ctx->scene_name);
    const RenderEngine renderEngine(ctx, scene);

    // Print Info
    ctx->PrintPasses();
    ctx->PrintRenderInfo();

    // Allocate output pass buffers
    for (auto &[fst, snd] : ctx->passes) {
        if ((snd.output == true) or (fst == "depth")) snd.data = OutputHandler::CreateOutputBuffer(ctx->width, ctx->height, snd.channels);
    }

    // Will be overwritten later, but just in case, init time
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();

    // Launch RenderEngines
    if (ctx->parallelism == Context::NONE) renderEngine.RenderSequential(start, end);
    else if (ctx->parallelism == Context::CPU) renderEngine.RenderParallelCPU(start, end);
    else GPUInitAndRender(scene.get(), ctx.get(), start, end);

    // Print Render Time
    const std::chrono::duration<double> seconds = end - start;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Render Time: " << seconds.count() << " seconds\n";

    // Save image to PPM
    for (auto &[fst, snd] : ctx->passes) {
        if (snd.output == true) {OutputHandler::WriteOutputBuffer(ctx->width, ctx->height, snd.channels, ctx->output_path, snd.name, snd.data); delete[] snd.data;}
        else if (fst == "depth") delete[] snd.data;
    }

    return EXIT_SUCCESS;
}