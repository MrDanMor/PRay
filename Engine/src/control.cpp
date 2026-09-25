#include "control.h"
#include "SDL3/SDL.h"

Controller::Controller() {
    mouseSensitivity = 0.005f;
}

Controller::~Controller() {
    
}

Rotation Controller::getCameraRotation() {
    SDL_Event event;
    
    return Rotation();
}

Vec3 Controller::getMovementDirection() {
    Vec3 movement = Vec3();

    const bool* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W]) movement.x += 1;
    if (keystate[SDL_SCANCODE_S]) movement.x -= 1;
    if (keystate[SDL_SCANCODE_A]) movement.z += 1;
    if (keystate[SDL_SCANCODE_D]) movement.z -= 1;
    if (keystate[SDL_SCANCODE_SPACE]) movement.y += 1;
    if (keystate[SDL_SCANCODE_LCTRL]) movement.y -= 1;

    return movement.normalized();
}

