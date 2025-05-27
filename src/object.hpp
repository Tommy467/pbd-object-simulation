#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "utils.hpp"

constexpr int32_t N = 5e5;
constexpr float GRAVITY = 40.0f;

struct Object {
    Object() = default;

#ifdef USE_CPU
    explicit Object(const float pos_x = 0.0f, const float pos_y = 0.0f, const float vel_x = 0.0f, const float vel_y = 0.0f, const float r = 0.5f, const float red = 255.0f, const float green = 255.0f, const float blue = 255.0f)
        : position_x(pos_x)
        , position_y(pos_y)
        , last_position_x(pos_x - vel_x)
        , last_position_y(pos_y - vel_y)
        , acceleration_x(0.0f)
        , acceleration_y(GRAVITY)
        , radius(r)
        , color_r(red)
        , color_g(green)
        , color_b(blue)
    {}

    float position_x{}, position_y{};
    float last_position_x{}, last_position_y{};
    float acceleration_x{}, acceleration_y{};
    float radius{};
    float color_r{}, color_g{}, color_b{};
#elif defined USE_GPU
    float position_x[N]{0.0f}, position_y[N]{0.0f};
    float last_position_x[N]{0.0f}, last_position_y[N]{0.0f};
    float radius[N]{0.5f};
    float color_r[N]{255.0f}, color_g[N]{255.0f}, color_b[N]{255.0f};
    int32_t size = 0;
    float acceleration_x = 0.0f, acceleration_y = GRAVITY;
    bool device_allocated = false;

    float *d_position_x = nullptr, *d_position_y = nullptr;
    float *d_last_position_x = nullptr, *d_last_position_y = nullptr;
    float *d_radius = nullptr;
#endif
};

#endif
