#include <iostream>
#include <cmath>

#include "PRAY.h"

int main(int argc, char* argv[]) {
    Vec3 center(0, 0, 5);
    Sphere* sphere = new Sphere(center, 1.0f);
    Renderer* prayRenderer = new Renderer("test", sphere);
    prayRenderer->start();

    SDL_Event e;
    bool keys[256] = { false };
    float mouseSensitivity = 0.002f;
    float moveSpeed = 5.0f;
    Uint32 lastTime = SDL_GetTicks();
    bool quit = false;

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
                prayRenderer->stop();
                break;
            }

            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.scancode < 256) keys[e.key.scancode] = true;
            }
            if (e.type == SDL_EVENT_KEY_UP) {
                if (e.key.scancode < 256) keys[e.key.scancode] = false;
            }

            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                prayRenderer->camera->rotation.addYaw(e.motion.xrel * mouseSensitivity);
                prayRenderer->camera->rotation.addPitch(-e.motion.yrel * mouseSensitivity);
            }
        }

        // Camera movement
        Vec3 forward;
        forward.x = sin(prayRenderer->camera->rotation.getYaw()) * cos(prayRenderer->camera->rotation.getPitch());
        forward.y = sin(prayRenderer->camera->rotation.getPitch());
        forward.z = cos(prayRenderer->camera->rotation.getYaw()) * cos(prayRenderer->camera->rotation.getPitch());
        forward = forward.normalized();
        Vec3 up = Vec3::UP();
        Vec3 right = forward.cross(up).normalized();

        const bool* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_W]) prayRenderer->camera->position += forward * moveSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_S]) prayRenderer->camera->position -= forward * moveSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_A]) prayRenderer->camera->position += right * moveSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_D]) prayRenderer->camera->position -= right * moveSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_SPACE]) prayRenderer->camera->position += up * moveSpeed * deltaTime;
        if (keystate[SDL_SCANCODE_LCTRL]) prayRenderer->camera->position -= up * moveSpeed * deltaTime;
    }

    return 0;
}