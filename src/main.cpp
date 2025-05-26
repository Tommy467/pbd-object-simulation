#include <SFML/Graphics/Font.hpp>
#include <iostream>

#include "fps_counter.hpp"
#include "physics_handler.hpp"
#include "random_number_generator.hpp"
#include "renderer.hpp"
#include "window_handler.hpp"

int main() {
    constexpr uint32_t window_width  = 1920;
    constexpr uint32_t window_height = 1080;
    const V2i world_size = {200, 200};

    WindowHandler window_handler("Test", sf::Vector2u(window_width, window_height));
    PhysicsHandler physics_handler({static_cast<float>(world_size.x), static_cast<float>(world_size.y)});
    Renderer renderer(physics_handler);
    constexpr float delta_time = 1.0f / 60.0f;

    constexpr float margin = 20.0f;
    const float zoom = static_cast<float>(window_height - margin) / static_cast<float>(world_size.y);
    window_handler.setZoom(zoom);
    window_handler.setFocus({static_cast<float>(world_size.x) * 0.5f, static_cast<float>(world_size.y) * 0.5f});

    bool isEmitting = true;
    window_handler.getEventManager().addKeyPressedCallback(sf::Keyboard::Space, [&](const sf::Event&) {
        isEmitting = !isEmitting;
    });

    int32_t emit_count = 10;
    window_handler.getEventManager().addKeyPressedCallback(sf::Keyboard::Down, [&](const sf::Event&) {
        emit_count = std::max(1, emit_count - 1);
    });
    window_handler.getEventManager().addKeyPressedCallback(sf::Keyboard::Up, [&](const sf::Event&) {
        emit_count = std::min(30, emit_count + 1);
    });

    int32_t rainbow_index = 0;
    constexpr int32_t rainbow_count = 1000;

    sf::Font font;
    font.loadFromFile("D:/Workspace/C++/PBD/res/times.ttf");
    sf::Clock clock;
    FPSCounter fps_counter;

    while (window_handler.run()) {

        if (isEmitting) {
            for (int i = emit_count; i > 0; i--) {
                float hue = static_cast<float>(rainbow_index) / static_cast<float>(rainbow_count);
                float r = 0, g = 0, b = 0;
                Renderer::HSVtoRGB(hue, 1.0f, 1.0f, r, g, b);
            #ifdef USE_CPU
                int obj_idx = physics_handler.createObject(2.0f, 5.0f + 1.5f * static_cast<float>(i), 0.2f, 0.0f, 0.5f, r, g, b);
                // int obj_idx = physics_handler.createObject(2.0f, 5.0f + 1.5f * static_cast<float>(i), 0.2f, 0.0f, RandomNumberGenerator::getFloat(0.2f, 1.0f), r, g, b);
            #elif defined USE_GPU
                int obj_idx = physics_handler.createObject(2.0f, 5.0f + 1.5f * static_cast<float>(i), 0.2f, 0.0f, 0.5f, r, g, b);
            #endif
            }
        }

        const float dt = clock.restart().asSeconds();
        fps_counter.update(dt);

        physics_handler.update(delta_time);

        window_handler.clear();
        renderer.render(window_handler);

        float fps = fps_counter.getFPS();
        int32_t object_count = physics_handler.getObjectsCount();

        window_handler.displayText(font, "FPS: " + std::to_string(static_cast<int>(fps)), {10.0f, 10.0f});
        window_handler.displayText(font, "Objects: " + std::to_string(object_count), {10.0f, 40.0f});
        window_handler.display();

        // if (fps <= 55.0f) {
        //     isEmitting = false;
        // }
        rainbow_index = (rainbow_index + 1) % rainbow_count;

    }

    return 0;
}
