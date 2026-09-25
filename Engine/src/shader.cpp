#include <string>
#include <fstream>

#include "SDL3/SDL.h"
#include "SDL3_shadercross/SDL_shadercross.h"



// SDL_GPUShader* compileShader(SDL_GPUDevice* device) {
//     std::ifstream file("raymarch.hlsl", std::ios::binary | std::ios::ate);
//     if (!file.is_open()) {
//         SDL_Log("Could not open raymarch.hlsl");
//         return nullptr;
//     }
//     size_t fileSize = file.tellg();
//     std::string hlslSource(fileSize, '\0');
//     file.seekg(0);
//     file.read(&hlslSource[0], fileSize);
//     if (!file) {
//         SDL_Log("Failed to read raymarch.hlsl");
//         return nullptr;
//     }

//     SDL_ShaderCross_HLSL_Info hlslInfo = {};
//     hlslInfo.source = hlslSource.c_str();
//     hlslInfo.entrypoint = "main";
//     hlslInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

//     // SDL_ShaderCross_GraphicsShaderMetadata metadata = {};
//     // metadata.num_inputs = 2;
//     // metadata.num_outputs = 0;
    
//     SDL_PropertiesID props = SDL_CreateProperties();
//     SDL_SetStringProperty(props, SDL_PROP_GPU_SHADER_CREATE_NAME_STRING, "RayMarching");

//     size_t spirvSize = 0;
//     void* spirvBytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &spirvSize);
//     if (!spirvBytecode) {
//         SDL_Log("Failed to compile HLSL to SPIR-V: ", SDL_GetError());
//         return nullptr;
//     }

//     SDL_ShaderCross_SPIRV_Info spirvInfo = {};
//     spirvInfo.bytecode = (const Uint8*) spirvBytecode;
//     spirvInfo.bytecode_size = spirvSize;
//     spirvInfo.entrypoint = "main";
//     spirvInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

//     SDL_ShaderCross_GraphicsShaderResourceInfo resourceInfo = {};
//     resourceInfo.num_uniform_buffers = 2;
//     resourceInfo.num_samplers = 0;
//     resourceInfo.num_storage_buffers = 0;
//     resourceInfo.num_storage_textures = 0;

//     SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
//         device,
//         &spirvInfo,
//         &resourceInfo,
//         props
//     );

//     SDL_free(spirvBytecode);
//     SDL_DestroyProperties(props);

//     if (!shader) {
//         SDL_Log("Failed to compile shader: %s", SDL_GetError());
//     }

//     return shader;
// }
