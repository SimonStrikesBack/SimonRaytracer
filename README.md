# Simon's Raytracer
Raytracing engine, written by Šimon Taněv for PG1, GPU and PCP courses at CTU.

## Directory Structure
* doc - Doxygen generated documentation
* output - example outputs from the provided config files
* scene - directory with obj and mtl scene files (including textures)
* src - source code of the project
* cmake_build.\[ps1/sh\] - CMake build script
* cmake_config.\[ps1/sh\] - CMake configure script
* config\[X\].json - provided config files for the raytracer
* Doxyfile - doxyfile config for documentation
* Raytracer.exe - provided Windows build of the raytracer in case compilation fails (filepaths in configs are tuned to its directory)
* README.md - this readme

## Compilation
Requirements:
* CMake
* CUDA Toolkit
* OpenMP
* C++20 compatible compiler (should be detected by CMake)

The build system used is CMake, which should automatically find both CUDA Toolkit and OpenMP if they are present.
Start by configuring the build directories by launching the cmake_config script (.ps1 for Windows, .sh for Linux).
Then build the project using the cmake_build script (.ps1 for Windows, .sh for Linux).
This will produce both a debug and a release build of the app.

If CMake asks for some extra program dependency, you may have to install it. This should not be the case generally, but if you use Arch btw. or have some other minimal OS with basically nothing on it, then it may not work right out of the gate.

If you are already on Windows, you can also just use the provided Raytracer.exe in the top folder.

## Running the Raytracer

Always launch the raytracer from a console, since it expects an argument to be passed.

Since the raytracer uses relative paths inside, it is advised to launch it from the same directory, since relative paths may not point where you think they do when launching from a different working directory.
It is easier to just modify the provided config files, rather than to build one from scratch.

If the app crashes during scene loading, it is most likely because of wrong file paths, double-check the path to the config and the paths inside the config.

All the relative paths in config files are set relative to the path of the provided Raytracer.exe in the top folder.

If you do not possess an Nvidia graphics card then the GPU rendering will not work and will probably fail silently (no Nvidia GPU is not a cuda error).
If your Nvidia graphics card is some exotic model with its architecture not being "86", then add your architecture to the CMakeLists.txt and recompile.

The example output of the provided config files is located in the ./output folder.

The raytracer supports binary format of BRDF materials used by UTIA CAS.
However, due to copyright which prohibits re-distribution without explicit consent, the material files themselves can not be shared and access to them must be given by UTIA.
You can ask for access at [this link](http://btf.utia.cas.cz/?brdf_dat_dwn).

Available arguments:
```
--help                  prints the help string (Help string shows the app arguments and the config file fields)
<path_to_config>        loads a config file at the specified path and renders with the config settings\
```

Config file fields (JSON):
```
width: int                          Width of the rendered image in pixels                                       
height: int                         Height of the rendered image in pixels                                      
pos: array[float]                   Position of camera in scene, 3 values (x,y,z)                               
up: array[float]                    Up vector, 3 values (x,y,z)                                                 
dir: array[float]                   Direction (Front) vector, 3 values (x,y,z)                                  
FOV: float                          FOV (Field of View)                                                         
scene_path: string                  Path to the folder that includes the scene (with '/' suffix)                
scene_name: string                  Name of the .obj scene file (with '.obj' suffix)
output_path: string                 Path to the folder where renders will be saved to (with '/' suffix)
area_light_samples: int             Number of samples to create when calculating area lights                    
max_recursion: int                  Number of ray bounces i.e. ray trace recursion calls                        
scale: int                          Factor to scale down the scene by (makes difference in light attenuation)   
BVH: bool                           Use BVH to accelerate ray traversal in scene                                
bvh_build_opt: bool                 Use optimizations to make BVH building faster                               
Parallelism: string                 Type of Parallelism to use ('CPU' | 'GPU' | 'NONE')                         
reflection: bool                    Calculate reflected ray shading                                             
refraction: bool                    Calculate refracted ray shading                                             
shadows: bool                       Test light visibility from point                                            
normal_mapping: bool                Bend normals according to normal maps (if present)                          
expensive_rays: bool                Perform expensive ray optimization on GPU rendering                         
expensive_threshold: int            Max recursion depth for a ray to not be considered expensive (GPU only)     
lights: array[JSONObject]           Array of point lights to use in scene (See light definition below)          
    position: array[float]          Position of point light in scene, 3 values (x,y,z)
    color: array[float]             Color of point light, 3 values (r,g,b)
passes: JSONObject                  Dictionary of which render passes to save (See available passes below)      
    combined: bool                  Final combined render 
    depth: bool                     Depth Buffer render 
    diffuse: bool                   Shaded diffuse render 
    specular: bool                  Shaded specular render 
    emissive: bool                  Shaded emissive render 
    material_diffuse: bool          Material diffuse color render 
    material_specular: bool         Material specular color render 
    reflection: bool                Reflection render 
    refraction: bool                Refraction render 
brdf_path: string                   Path to the folder that includes BRDF materials (.bin files)                
brdf_override: array[JSONObject]    Array of material overrides to use BRDF instead of standard diffuse and specular        
    <material_name>: string         Mapping of material name to a string that points to a BRDF (with .bin suffix)
```

## Attribution
* Code for BRDF loading and sampling was provided by UTIA CAS, available at [this link](http://btf.utia.cas.cz/?brdf_dat_dwn).
* Code for loading of png files was created by Lode Vandevenne, available at [this link](https://github.com/lvandeve/lodepng).
* Code for JSON parsing was created by Niels Lohmann, available at [this link](https://github.com/nlohmann/json).
* Code for obj and mtl parsing Syoyo Fujita and other contributors, available at [this link](https://github.com/tinyobjloader/tinyobjloader).
* Some of the provided scenes were created by Guedis Cardenas and Morgan McGuire, original link to their work does not work anymore