#pragma once

#include "core.h"

class PRAY_API EventListener {
public:
    virtual ~EventListener() = default;
};

class PRAY_API EventPublisher {
public:
    EventPublisher();
};

class PRAY_API Event {
    ~Event() = default;
};

class PRAY_API WindowResizeEvent : public Event {
public:
    //WindowResizeEvent(int newWidth, int newHeight, SDL_Window* window);

    unsigned int getNewWidth() const { return newWidth; }
    unsigned int getNewHeight() const { return newHeight; }

    unsigned int getOldWidth() const { return oldWidth; }
    unsigned int getOldHeight() const { return oldHeight; }

private:
    unsigned int newWidth, newHeight, oldWidth, oldHeight;
};