#pragma once

#include "core.h"

class PRAY_API Controller {
public:
    float mouseSensitivity;
    float moveSpeed;

    Controller();
    ~Controller();

    Vec3 getMovementDirection();
    Rotation getCameraRotation();

private:
    
};