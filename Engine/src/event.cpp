// #include "event.h"
// #include "SDL3/SDL.h"

// WindowResizeEvent::WindowResizeEvent(
//     int newWidth,
//     int newHeight,
//     SDL_Window* window
// ) {
//     this->newWidth = newWidth;
//     this->newHeight = newHeight;

//     int* w;
//     int* h;

//     SDL_GetWindowSizeInPixels(window, w, h);

//     oldWidth = (unsigned int) w;
//     oldHeight = (unsigned int) h;
// }

// EventPublisher::EventPublisher() {
    
// }

// void tick() {
//     SDL_Event sdlEvent;
//     while (SDL_PollEvent(&sdlEvent) != 0) {
//         if (sdlEvent.type == SDL_EVENT_QUIT) {
            
//         }

//         if (sdlEvent.type == SDL_EVENT_WINDOW_RESIZED) {
//             // WindowResizeEvent* event = WindowResizeEvent(
                
//             // );
//         }

//             // if (e.type == SDL_EVENT_KEY_DOWN) {
//             //     if (e.key.scancode < 256) keys[e.key.scancode] = true;
//             // }
//             // if (e.type == SDL_EVENT_KEY_UP) {
//             //     if (e.key.scancode < 256) keys[e.key.scancode] = false;
//             // }

//             // if (e.type == SDL_EVENT_MOUSE_MOTION) {
//             //     prayRenderer->cameraRot.yaw += e.motion.xrel * mouseSensitivity;
//             //     prayRenderer->cameraRot.pitch -= e.motion.yrel * mouseSensitivity;
//             // }
//     }
// }