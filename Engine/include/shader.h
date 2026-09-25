#include "SDL3/SDL.h"

// SDL_GPUShader* compileShader(SDL_GPUDevice* device);



struct SphereData {
    float centerRadius[4];
    float lightDirection[4];

    void setCenter(Vec3 center) {
        this->centerRadius[0] = center.x;
        this->centerRadius[1] = center.y;
        this->centerRadius[2] = center.z;
    }

    void setRadius(float radius) {
        this->centerRadius[3] = radius;
    }

    void setLightDirection(Vec3 direction) {
        this->lightDirection[0] = direction.x;
        this->lightDirection[1] = direction.y;
        this->lightDirection[2] = direction.z;
        this->lightDirection[3] = 0.0f;
    }
};

struct CameraData {
    float position[4];
    float movement[4];
    float rotationAndAspect[4];

    void setPosition(Vec3 position) {
        this->position[0] = position.x;
        this->position[1] = position.y;
        this->position[2] = position.z;
        this->position[3] = 0.0f;
    }

    void setMovement(Vec3 movement) {
        this->movement[0] = movement.x;
        this->movement[1] = movement.y;
        this->movement[2] = movement.z;
        this->movement[3] = 0.0f;
    }

    void setRotation(Rotation rotation) {
        this->rotationAndAspect[0] = rotation.getYaw();
        this->rotationAndAspect[1] = rotation.getPitch();
        this->rotationAndAspect[2] = 0.0f;
        this->rotationAndAspect[3] = 0.0f;
    }

    void setAspect(float aspect) {
        this->rotationAndAspect[2] = aspect;
    }
};