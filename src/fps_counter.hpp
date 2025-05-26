#ifndef FPS_COUNTER_HPP
#define FPS_COUNTER_HPP

#include <SFML/System/Clock.hpp>

class FPSCounter {
public:
    FPSCounter()
        : frame_count(0)
        , elapsed_time(0.0f)
        , fps(0.0f)
    {}

    void update(float delta_time) {
        elapsed_time += delta_time;
        ++frame_count;

        if (elapsed_time >= 1.0f) {
            fps = static_cast<float>(frame_count) / elapsed_time;
            elapsed_time = 0.0f;
            frame_count = 0;
        }
    }

    [[nodiscard]]
    float getFPS() const {
        return fps;
    }

private:
    sf::Clock clock;
    int frame_count;
    float elapsed_time;
    float fps;
};

#endif
