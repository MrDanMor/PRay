#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <filesystem>

#include "render.h"
#include "SDL3/SDL.h"
#include "SDL3_shadercross/SDL_shadercross.h"
#include "shader.h"

SDL_GPUTexture *createRenderTexture(Renderer *renderer)
{
    SDL_GPUTextureCreateInfo texInfo = {};
    texInfo.type = SDL_GPU_TEXTURETYPE_2D;
    texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texInfo.width = renderer->width;
    texInfo.height = renderer->height;
    texInfo.layer_count_or_depth = 1;
    texInfo.num_levels = 1;
    texInfo.usage =
        SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ |
        SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE |
        SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture *outputTexture = SDL_CreateGPUTexture(renderer->gpu, &texInfo);
    if (!outputTexture)
    {
        std::cerr << "Failed to create output texture: " << SDL_GetError() << std::endl;
        throw std::invalid_argument("outputTexture is nullptr");
    }

    return outputTexture;
}

static std::string resolveShaderPath(const std::string &relativePath)
{
    std::vector<std::string> candidates;
    candidates.push_back("Engine/shaders/" + relativePath);
    candidates.push_back("Sandbox/" + relativePath);
    candidates.push_back(relativePath);

    const char *basePath = SDL_GetBasePath();
    if (basePath)
    {
        candidates.push_back(std::string(basePath) + "Engine/shaders/" + relativePath);
        candidates.push_back(std::string(basePath) + "Sandbox/" + relativePath);
        candidates.push_back(std::string(basePath) + relativePath);
    }

    for (const auto &candidate : candidates)
    {
        std::ifstream probe(candidate.c_str(), std::ios::binary | std::ios::ate);
        if (probe.is_open())
        {
            return candidate;
        }
    }

    return {};
}

static std::string getDXCProfile(SDL_ShaderCross_ShaderStage stage)
{
    switch (stage)
    {
    case SDL_SHADERCROSS_SHADERSTAGE_VERTEX:
        return "vs_6_0";
    case SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT:
        return "ps_6_0";
    case SDL_SHADERCROSS_SHADERSTAGE_COMPUTE:
        return "cs_6_0";
    default:
        return "cs_6_0";
    }
}

static bool compileHLSLToSPIRVFromFile(const std::string &resolvedPath, SDL_ShaderCross_ShaderStage stage, std::vector<Uint8> &outBytes)
{
    const std::filesystem::path sourcePath = std::filesystem::absolute(resolvedPath);
    std::filesystem::path chosenPath;
    std::vector<std::filesystem::path> searchRoots;

    const char *basePath = SDL_GetBasePath();
    if (basePath)
    {
        std::filesystem::path baseRoot(basePath);
        for (int i = 0; i < 12 && !baseRoot.empty(); ++i)
        {
            searchRoots.push_back(baseRoot);
            baseRoot = baseRoot.parent_path();
        }
    }

    std::filesystem::path searchRoot = sourcePath.parent_path();
    for (int i = 0; i < 12 && !searchRoot.empty(); ++i)
    {
        searchRoots.push_back(searchRoot);
        searchRoot = searchRoot.parent_path();
    }

    for (const auto &root : searchRoots)
    {
        const auto candidate = root / "generated_shaders" / (sourcePath.stem().string() + ".spv");
        if (std::filesystem::exists(candidate))
        {
            chosenPath = candidate;
            break;
        }
    }

    if (chosenPath.empty())
    {
        chosenPath = std::filesystem::path("generated_shaders") / (sourcePath.stem().string() + ".spv");
    }
    if (!std::filesystem::exists(chosenPath))
    {
        std::vector<std::string> dxcCandidates = {
            "Engine/libs/DXC/bin/x64/dxc.exe",
            "C:/Users/danil/Desktop/GameDev/PRay/Engine/libs/DXC/bin/x64/dxc.exe",
            "C:/msys64/mingw64/bin/dxc.exe"
        };

        std::string dxcPath;
        for (const auto &candidate : dxcCandidates)
        {
            std::filesystem::path candidatePath(candidate);
            if (std::filesystem::exists(candidatePath))
            {
                dxcPath = std::filesystem::absolute(candidatePath).string();
                break;
            }
        }

        if (dxcPath.empty())
        {
            SDL_Log("Could not locate dxc.exe for shader compilation.");
            return false;
        }

        const std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        chosenPath = tempDir / (sourcePath.stem().string() + ".spv");
        const std::string command =
            "powershell -NoLogo -NoProfile -Command \"& '" + dxcPath + "' -spirv -T " + getDXCProfile(stage) + " -E main -Fo '" + chosenPath.string() + "' '" + sourcePath.string() + "'\"";
        const int exitCode = std::system(command.c_str());
        if (exitCode != 0)
        {
            SDL_Log("dxc failed for %s (exit %d)", resolvedPath.c_str(), exitCode);
            return false;
        }
    }

    std::ifstream spvFile(chosenPath, std::ios::binary | std::ios::ate);
    SDL_Log("Attempting to read SPIR-V from %s", chosenPath.string().c_str());
    if (!spvFile.is_open())
    {
        SDL_Log("Failed to open generated SPIR-V file: %s", chosenPath.string().c_str());
        return false;
    }

    const std::streamsize fileSize = spvFile.tellg();
    if (fileSize <= 0)
    {
        SDL_Log("Generated SPIR-V file was empty: %s", chosenPath.string().c_str());
        return false;
    }

    outBytes.resize(static_cast<size_t>(fileSize));
    spvFile.seekg(0);
    spvFile.read(reinterpret_cast<char *>(outBytes.data()), fileSize);
    spvFile.close();

    return true;
}

ShaderCodeInfo compileShader(
    std::string path,
    SDL_ShaderCross_ShaderStage shaderStage,
    const char *entrypoint = "main")
{
    const std::string resolvedPath = resolveShaderPath(path);
    if (resolvedPath.empty())
    {
        SDL_Log("Could not open hlsl file: %s", path.c_str());
        return {};
    }

    ShaderCodeInfo compiled = {};
    compiled.entrypoint = entrypoint;
    if (!compileHLSLToSPIRVFromFile(resolvedPath, shaderStage, compiled.bytecode_storage))
    {
        SDL_Log("SPIR-V compilation failed for: %s", resolvedPath.c_str());
        return {};
    }

    compiled.bytecode = compiled.bytecode_storage.data();
    compiled.size = compiled.bytecode_storage.size();
    return compiled;
}

SDL_GPUComputePipelineCreateInfo setupComputeShader()
{
    ShaderCodeInfo shader = compileShader("raymarch.hlsl", SDL_SHADERCROSS_SHADERSTAGE_COMPUTE);

    SDL_GPUComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.code = shader.bytecode;
    pipelineInfo.code_size = shader.size;
    pipelineInfo.entrypoint = shader.entrypoint;
    pipelineInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    pipelineInfo.num_uniform_buffers = 2;
    pipelineInfo.num_readwrite_storage_textures = 1;
    pipelineInfo.threadcount_x = 16;
    pipelineInfo.threadcount_y = 16;
    pipelineInfo.threadcount_z = 1;

    return pipelineInfo;
}

SDL_GPUGraphicsPipelineCreateInfo setupGraphicsShader(Renderer *renderer)
{
    std::cerr << "[graphics] compiling vertex.hlsl\n";
    ShaderCodeInfo vertexCode =
        compileShader("vertex.hlsl", SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
    std::cerr << "[graphics] vertex bytecode size=" << vertexCode.size << "\n";
    std::cerr << "[graphics] compiling fragment.hlsl\n";
    ShaderCodeInfo fragmentCode =
        compileShader("fragment.hlsl", SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
    std::cerr << "[graphics] fragment bytecode size=" << fragmentCode.size << "\n";
    if (vertexCode.bytecode == nullptr || fragmentCode.bytecode == nullptr)
    {
        SDL_Log("Unable to compile one or more source HLSL files; aborting graphics setup.");
        SDL_GPUGraphicsPipelineCreateInfo emptyInfo = {};
        return emptyInfo;
    }

    SDL_GPUShaderCreateInfo vertexInfo = {};
    vertexInfo.code = vertexCode.bytecode;
    vertexInfo.code_size = vertexCode.size;
    vertexInfo.entrypoint = vertexCode.entrypoint;
    vertexInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vertexInfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vertexInfo.num_uniform_buffers = 0;
    vertexInfo.num_samplers = 0;
    vertexInfo.num_storage_textures = 0;
    vertexInfo.num_storage_buffers = 0;

    SDL_GPUShaderCreateInfo fragmentInfo = {};
    fragmentInfo.code = fragmentCode.bytecode;
    fragmentInfo.code_size = fragmentCode.size;
    fragmentInfo.entrypoint = fragmentCode.entrypoint;
    fragmentInfo.num_samplers = 1;
    fragmentInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragmentInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;

    std::cerr << "[graphics] creating vertex shader\n";
    SDL_GPUShader *vertexShader = SDL_CreateGPUShader(renderer->gpu, &vertexInfo);
    std::cerr << "[graphics] vertex shader ptr=" << (void *)vertexShader << "\n";
    std::cerr << "[graphics] creating fragment shader\n";
    SDL_GPUShader *fragmentShader = SDL_CreateGPUShader(renderer->gpu, &fragmentInfo);
    std::cerr << "[graphics] fragment shader ptr=" << (void *)fragmentShader << "\n";

    if (!vertexShader || !fragmentShader)
    {
        SDL_Log("Shader creation failed: %s", SDL_GetError());
    }

    static SDL_GPUColorTargetDescription colorDescription = {};
    colorDescription.format = SDL_GetGPUSwapchainTextureFormat(renderer->gpu, renderer->window);

    SDL_GPUColorTargetBlendState blendState = {};
    blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
    blendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
    blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
    blendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    blendState.color_write_mask =
        SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G |
        SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;
    blendState.enable_blend = false;
    blendState.enable_color_write_mask = true;
    colorDescription.blend_state = blendState;

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipelineInfo.depth_stencil_state.enable_depth_test = false;
    pipelineInfo.depth_stencil_state.enable_depth_write = false;
    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = &colorDescription;
    pipelineInfo.target_info.has_depth_stencil_target = false;
    pipelineInfo.vertex_input_state =
        SDL_GPUVertexInputState{nullptr, 0, nullptr, 0};

    return pipelineInfo;
}

Renderer::Renderer(const char *title, RenderedObject *object) :
    width(800), height(600),
    m_title(title), m_running(false), m_quit(false)
{
    std::shared_ptr<RenderedObject> shared_ptr(object);
    this->m_object = shared_ptr;
}

Renderer::~Renderer()
{
    stop();
    if (m_thread && m_thread->joinable())
        m_thread->join();
}

void Renderer::start()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL error: " << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_ShaderCross_Init())
    {
        std::cerr << "ShaderCross init failed: " << SDL_GetError() << std::endl;
        return;
    }

    this->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    if (!gpu)
    {
        std::cerr << "SDL GPU error: " << SDL_GetError() << std::endl;
        return;
    }

    this->window = SDL_CreateWindow("PRAY", width, height, SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "SDL window error: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_ShowWindow(window);
    SDL_ShowWindow(window);
    SDL_ClaimWindowForGPUDevice(gpu, window);
    SDL_SetWindowRelativeMouseMode(window, true);

    CameraData cameraData = {};
    cameraData.setPosition(Vec3(0, 0, -3));
    cameraData.setRotation(Rotation());
    cameraData.setMovement(Vec3());
    cameraData.setAspect(1.0f);
    this->cameraBuffer = new StaticReadBuffer(gpu, &cameraData, sizeof(CameraData));

    SphereData sphereData = {};
    sphereData.setLightDirection(Vec3(1, 1, 0).normalized());
    sphereData.setCenter(Vec3(0, 0, 3));
    sphereData.setRadius(1.0f);
    this->sphereBuffer = new StaticReadBuffer(gpu, &sphereData, sizeof(SphereData));

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
    if (!cmd)
    {
        SDL_Log("Failed to acquire GPU command buffer: %s", SDL_GetError());
        return;
    }

    ShaderCodeInfo computeShader =
        compileShader("raymarch.hlsl", SDL_SHADERCROSS_SHADERSTAGE_COMPUTE);
    this->computeShaderCode = computeShader.bytecode_storage;
    SDL_Log("Compute shader bytecode size: %zu", computeShader.bytecode_storage.size());

    SDL_GPUComputePipelineCreateInfo computeInfo = setupComputeShader();
    std::cerr << "[start] creating compute pipeline\n";
    this->computePipeline = SDL_CreateGPUComputePipeline(gpu, &computeInfo);
    if (!this->computePipeline)
    {
        SDL_Log("Compute pipeline creation failed! SDL_GetError: %s", SDL_GetError());
        return;
    }

    std::cerr << "[start] creating graphics pipeline\n";
    SDL_GPUGraphicsPipelineCreateInfo graphicsInfo = setupGraphicsShader(this);
    std::cerr << "[start] before SDL_CreateGPUGraphicsPipeline\n";
    auto swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu, window);
    std::cerr << "[start] swapchain format = " << static_cast<int>(swapchainFormat) << "\n";
    this->graphicsPipeline = SDL_CreateGPUGraphicsPipeline(gpu, &graphicsInfo);
    if (!graphicsPipeline)
    {
        SDL_Log("Pipeline creation failed! SDL_GetError: %s", SDL_GetError());
        std::cerr << "[start] pipeline creation failed! SDL_GetError: " << SDL_GetError() << "\n";
        return;
    }

    this->texture = createRenderTexture(this);

    // A plain SDL renderer is not needed for the GPU-only render path.
    // if (!m_running) {
    //     m_running = true;
    //     m_thread = std::make_unique<std::thread>(run, this);
    // }

    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    this->sampler = SDL_CreateGPUSampler(gpu, &samplerInfo);

    if (!this->sampler)
    {
        SDL_Log("Failed to create GPU sampler: %s", SDL_GetError());
        return;
    }

    run();
}

void Renderer::stop()
{
    m_quit = true;
}

void Renderer::wait()
{
    if (m_thread && m_thread->joinable()) m_thread->join();
}

int Renderer::run()
{
    Uint32 groupX = (width + 15) / 16;
    Uint32 groupY = (height + 15) / 16;

    while (!m_quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            // if (e.type == SDL_EVENT_QUIT)
            // {
            //     m_quit = true;
            //     break;
            // }

            if (e.type == SDL_EVENT_WINDOW_RESIZED)
            {
                width = static_cast<Uint32>(e.window.data1);
                height = static_cast<Uint32>(e.window.data2);

                if (width == 0 || height == 0)
                    continue;

                // Replace with src lifetime management later
                SDL_WaitForGPUIdle(gpu);

                if (texture)
                {
                    SDL_ReleaseGPUTexture(gpu, texture);
                    texture = nullptr;
                }

                texture = createRenderTexture(this);
                groupX = (width + 15) / 16;
                groupY = (height + 15) / 16;
            }
        }

        if (width == 0 || height == 0)
            continue;

        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);

        if (!cmd)
        {
            SDL_Log("SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            continue;
        }

        SphereData sphere = {};
        sphere.setCenter(Vec3(0.0f, 0.0f, 3.0f));
        sphere.setRadius(1.0f);
        sphere.setLightDirection(Vec3(1.0f, 1.0f, 0.0f).normalized());

        CameraData camera = {};
        camera.setPosition(Vec3(0.0f, 0.0f, -3.0f));
        camera.setMovement(Vec3());
        camera.setRotation(Rotation());
        camera.setAspect(static_cast<float>(width) / static_cast<float>(height));

        SDL_PushGPUComputeUniformData(cmd, 0, &sphere, sizeof(SphereData));
        SDL_PushGPUComputeUniformData(cmd, 1, &camera, sizeof(CameraData));

        SDL_GPUStorageTextureReadWriteBinding storageBinding = {};
        storageBinding.texture = texture;
        storageBinding.cycle = false;

        SDL_GPUComputePass *computePass =
            SDL_BeginGPUComputePass(cmd, &storageBinding, 1, nullptr, 0);

        if (!computePass)
        {
            SDL_Log("SDL_BeginGPUComputePass failed: %s", SDL_GetError());
            SDL_CancelGPUCommandBuffer(cmd);
            continue;
        }

        SDL_BindGPUComputePipeline(computePass, computePipeline);
        SDL_DispatchGPUCompute(computePass, groupX, groupY, 1);
        SDL_EndGPUComputePass(computePass);

        SDL_GPUTexture *swapchainTexture = nullptr;
        Uint32 swapchainWidth = 0;
        Uint32 swapchainHeight = 0;

        const bool acquired = SDL_WaitAndAcquireGPUSwapchainTexture(
            cmd, window, &swapchainTexture,&swapchainWidth,&swapchainHeight
        );

        if (!acquired)
        {
            SDL_Log("SDL_WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            SDL_CancelGPUCommandBuffer(cmd);
            continue;
        }

        if (!swapchainTexture)
        {
            SDL_CancelGPUCommandBuffer(cmd);
            continue;
        }

        SDL_GPUColorTargetInfo colorTarget = {};
        colorTarget.texture = swapchainTexture;
        colorTarget.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(
            cmd, &colorTarget, 1, nullptr
        );

        if (!renderPass)
        {
            // Swapchain texture acquired, cannot cancel command buffer
            SDL_Log("SDL_BeginGPURenderPass failed: %s", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(cmd);
            continue;
        }

        SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);
        SDL_GPUViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.w = static_cast<float>(swapchainWidth != 0 ? swapchainWidth : width);
        viewport.h = static_cast<float>(swapchainHeight != 0 ? swapchainHeight : height);
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;

        SDL_SetGPUViewport(renderPass, &viewport);
        SDL_Rect scissor = {};
        scissor.x = 0;
        scissor.y = 0;
        scissor.w = static_cast<int>(swapchainWidth != 0 ? swapchainWidth : width);
        scissor.h = static_cast<int>(swapchainHeight != 0 ? swapchainHeight : height);

        SDL_SetGPUScissor(renderPass, &scissor);
        SDL_GPUTextureSamplerBinding textureBinding = {};
        textureBinding.texture = texture;
        textureBinding.sampler = sampler;

        SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);
        SDL_DrawGPUPrimitives(
            renderPass,
            3, // vertex count
            1, // instance count
            0, // first vertex
            0  // first instance
        );

        SDL_EndGPURenderPass(renderPass);
        if (!SDL_SubmitGPUCommandBuffer(cmd))
        {
            SDL_Log("SDL_SubmitGPUCommandBuffer failed: %s", SDL_GetError());
        }
    }

    return 0;
}