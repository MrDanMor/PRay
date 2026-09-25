#pragma once

// Define export/import macros
#ifdef _WIN32
    #define PRAY_API __declspec(dllexport)
    // #ifdef PRAY_EXPORTS
    //     #define PRAY_API __declspec(dllexport)
    // #else
    //     #define PRAY_API __declspec(dllimport)
    // #endif
#else // Linux/MacOS
    #define PRAY_API
#endif



#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI M_PI

#include <cmath>



template <typename T>
T PRAY_API clamp(T x, T min, T max);



template <typename T>
bool PRAY_API isInRange(T x, T a, T b) {
    return (x >= a && x <= b) ? true : false;
}

struct PRAY_API Vec3 {
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3 operator + (const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator += (const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3 operator - (const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator -= (const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

    Vec3 operator * (float a) const { return Vec3(x * a, y * a, z * a); }

    float dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }

    Vec3 normalized() const { return *this * (1.0f / length()); }
    Vec3 cross(const Vec3& v) const {
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    static Vec3 UP() { return Vec3(0, 1, 0); }
};

struct PRAY_API Rotation {
private:
    float yaw, pitch;
    
public:
    Rotation(float yaw = 0, float pitch = 0) : yaw(yaw), pitch(pitch) {}

    float getYaw() const { return yaw; }
    void setYaw(const float& yaw);
    void addYaw(const float& deltaYaw) { setYaw(yaw + deltaYaw); }
    
    float getPitch() const { return pitch; }
    void setPitch(const float& pitch);
    void addPitch(const float& deltaPitch) { setPitch(pitch + deltaPitch); }
};