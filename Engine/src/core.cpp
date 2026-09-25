#include "core.h"
#include "SDL3/SDL_stdinc.h"

template <typename T>
T clamp(T x, T min, T max) {
    return SDL_clamp(x, min, max);
}

void Rotation::setYaw(const float& yaw) {
    this->yaw = std::fmod(yaw, static_cast<float>(PI) * 2.0f);
}

void Rotation::setPitch(const float& pitch) {
    this->pitch = SDL_clamp(pitch, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
}