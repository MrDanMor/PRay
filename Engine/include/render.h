#include <cmath>
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <vector>

#include "core.h"
#include "SDL3/SDL.h"

struct ShaderCodeInfo {
    std::vector<Uint8> bytecode_storage;
    const Uint8* bytecode = nullptr;
    size_t size = 0;
    const char* entrypoint = "main";
};

class PRAY_API RenderedObject {
public:
    Vec3 position;
    virtual float surfaceDistanceFunction(const Vec3& point) = 0;
    virtual Vec3 normalFunction(const Vec3& point) = 0;
    virtual ~RenderedObject() = default;
};

class PRAY_API Sphere : public RenderedObject {
public:
    float radius;
    Sphere(const Vec3& position, float radius) {
        this->position = position;
        this->radius = radius;
    }

    float surfaceDistanceFunction(const Vec3& point) override {
        return (point - position).length() - radius;
    }

    Vec3 normalFunction(const Vec3& point) override {
        return (point - position).normalized();
    }
};

class PRAY_API Camera {
public:
    Vec3 position;
    Rotation rotation;

    float mouseSensitivity = 0.01f;
    float moveSpeed = 5.0f;
};



struct StaticReadBuffer {
public:
    StaticReadBuffer(SDL_GPUDevice* device, const void* bufferStructure, size_t structureSize) {
        SDL_GPUBufferCreateInfo info = {};
        info.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;
        info.size = structureSize;
        this->buffer = SDL_CreateGPUBuffer(device, &info);

        SDL_GPUTransferBufferCreateInfo transferInfo = {};
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferInfo.size = structureSize;
        this->transfer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
        void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
        memcpy(mapped, bufferStructure, structureSize);
        SDL_UnmapGPUTransferBuffer(device, transfer);
        this->location = { transfer, 0 };
        this->region = { buffer, 0, static_cast<Uint32>(structureSize) };
    }

    SDL_GPUBuffer* buffer;
    SDL_GPUTransferBuffer* transfer;
    SDL_GPUTransferBufferLocation location;
    SDL_GPUBufferRegion region;
};

class PRAY_API Renderer {
public:
    Renderer(const char* title, RenderedObject* object);
    ~Renderer();

    void start();
    void stop();
    void wait();

    Camera* camera;

    int width, height;
    SDL_Window* window;
    SDL_GPUDevice* gpu;
    SDL_GPUTexture* texture;
    SDL_Renderer* renderer;

private:
    SDL_GPUSampler* sampler;
    std::string m_title;
    std::shared_ptr<RenderedObject> m_object;
    std::atomic<bool> m_running;
    std::atomic<bool> m_quit;
    std::unique_ptr<std::thread> m_thread;

    SDL_GPUComputePipeline* computePipeline;
    SDL_GPUGraphicsPipeline* graphicsPipeline;
    std::vector<Uint8> computeShaderCode;
    StaticReadBuffer* cameraBuffer;
    StaticReadBuffer* sphereBuffer;

    int run();

    float precision = 0.001f;
    float max_distance = 100.0f;
    int cast_depth = 128;

    bool setPrecision(float precision) {
        if (isInRange(precision, 0.f, 1.f)) {
            this->precision = precision;
            return true;
        }

        return false;
    }

    bool setRenderDistance(float distance) {
        if (isInRange(distance, 0.f, 1.f)) {
            this->max_distance = distance;
            return true;
        }

        return false;
    }
};

