cmake_minimum_required(VERSION 3.10)
project(Raytracer LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES "86")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
#set(CMAKE_CUDA_ARCHITECTURES native)

find_package(CUDAToolkit REQUIRED)
find_package(OpenMP REQUIRED COMPONENTS CXX)

#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-arch=sm_61)
set_source_files_properties(kernel.cu PROPERTIES LANGUAGE CUDA)
add_executable(Raytracer main.cpp vec3.hpp constants.hpp geomath.hpp ray.hpp context.hpp context.cpp vec3.cpp vertex.hpp geomath.cpp triangle.cpp triangle.hpp scene.cpp scene.hpp material.hpp object.hpp light.hpp render_engine.cpp render_engine.hpp
        OutputHandler.cpp
        OutputHandler.hpp
        point_light.hpp
        area_light.hpp
        bounding_box.cpp
        bounding_box.hpp
        bvh_node.cpp
        bvh_node.hpp
        vec2.cpp
        vec2.hpp
        include/lodepng.cpp
        include/TBRDF.cpp
        texture.hpp
        mat3.hpp
        kernel.cu
        gpu_code/gpu_vec2.cu
        gpu_code/gpu_vec2.cuh
        gpu_code/gpu_vec3.cu
        gpu_code/gpu_vec2.cuh
        gpu_code/gpu_vertex.cuh
        gpu_code/gpu_triangle.cuh
        gpu_code/gpu_ray.cuh
        gpu_code/gpu_material.cuh
        gpu_code/gpu_texture.cuh
        gpu_code/gpu_texture.cuh
        gpu_code/gpu_mat3.cuh
        gpu_code/gpu_geomath.cuh
        gpu_code/gpu_geomath.cu
        gpu_code/gpu_triangle.cu
        gpu_code/gpu_object.cuh
        gpu_code/gpu_point_light.cuh
        gpu_code/gpu_area_light.cuh
        gpu_code/gpu_bounding_box.cuh
        gpu_code/gpu_bounding_box.cu
        gpu_code/gpu_bvh_node.cuh
        gpu_code/gpu_context.cuh
        gpu_code/gpu_scene.cuh
        gpu_code/gpu_render_engine.cuh
        gpu_code/gpu_render_engine.cu
        gpu_code/gpu_rand.cuh
        gpu_code/gpu_BRDF.cuh
        gpu_code/gpu_pair.cuh
)

set_target_properties(Raytracer PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        CUDA_EXTENSIONS OFF
)

target_compile_options(Raytracer PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:/openmp:llvm>
)

target_link_libraries(Raytracer PRIVATE CUDA::cudart)
target_link_libraries(Raytracer PRIVATE OpenMP::OpenMP_CXX)

# Include CUDA headers
target_include_directories(Raytracer PRIVATE
        ${CUDA_INCLUDE_DIRS}
)


target_include_directories(Raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gpu_code)
target_include_directories(Raytracer PRIVATE
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)
