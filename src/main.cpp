#include <SFML/Graphics/Font.hpp>
#include <iostream>
#include <fstream>

#include "fps_counter.hpp"
#include "physics_handler.hpp"
#include "random_number_generator.hpp"
#include "renderer.hpp"
#include "window_handler.hpp"

#ifdef OUTPUT_RESULTS
std::ofstream output_file;
#endif
int32_t particle_min_count = 0;
int32_t particle_max_count = 25e4;

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

    int32_t emit_count = 20;
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

#ifdef OUTPUT_RESULTS
#ifdef USE_CPU
    std::string path = "D:/Workspace/C++/PBD/result/cpu_threads" + std::to_string(cpu_threads) + ".csv";
    output_file.open(path);
    output_file << "object_counts,physics_update_elapsed_time,render_elapsed_time\n";
#elif defined USE_GPU
    std::string path = "D:/Workspace/C++/PBD/result/gpu_block_size" + std::to_string(gpu_block_size) + ".csv";
    output_file.open(path);
    output_file << "object_counts,gpu_elapsed_time,physics_update_elapsed_time,render_elapsed_time\n";
#endif
#endif

    while (window_handler.run()) {
        if (isEmitting && physics_handler.getObjectsCount() < particle_max_count) {
            for (int i = emit_count; i > 0; i--) {
                float hue = static_cast<float>(rainbow_index) / static_cast<float>(rainbow_count);
                float r = 0, g = 0, b = 0;
                Renderer::HSVtoRGB(hue, 1.0f, 1.0f, r, g, b);
                const float vel_x  = RandomNumberGenerator::getFloat(0.05f, 0.1f);
                const float vel_y  = RandomNumberGenerator::getFloat(-0.1f, 0.1f);
                const float radius = RandomNumberGenerator::getFloat(0.2f, 0.5f);
            #ifdef USE_CPU
                int obj_idx = physics_handler.createObject(2.0f, 5.0f + 1.5f * static_cast<float>(i), vel_x, vel_y, radius, r, g, b);
            #elif defined USE_GPU
                int obj_idx = physics_handler.createObject(2.0f, 5.0f + 1.5f * static_cast<float>(i), vel_x, vel_y, radius, r, g, b);
            #endif
            }
        }

        const float dt = clock.restart().asSeconds();
        fps_counter.update(dt);

    #ifdef OUTPUT_RESULTS
        if (physics_handler.getObjectsCount() > particle_min_count) {
            output_file << physics_handler.getObjectsCount() << ",";
        }

        auto physics_update_start = std::chrono::high_resolution_clock::now();
    #endif
        physics_handler.update(delta_time);
    #ifdef OUTPUT_RESULTS
        auto physics_update_end = std::chrono::high_resolution_clock::now();
        auto physics_update_duration = std::chrono::duration_cast<std::chrono::microseconds>(physics_update_end - physics_update_start).count();
        if (physics_handler.getObjectsCount() > particle_min_count) {
            output_file << physics_update_duration << ",";
        }
        auto render_start = std::chrono::high_resolution_clock::now();
    #endif
        window_handler.clear();
        renderer.render(window_handler);
    #ifdef OUTPUT_RESULTS
        auto render_end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(render_end - render_start).count();
        if (physics_handler.getObjectsCount() > particle_min_count) {
            output_file << duration << "\n";
        }
    #endif

        float fps = fps_counter.getFPS();
        int32_t object_count = physics_handler.getObjectsCount();

        window_handler.displayText(font, "FPS: " + std::to_string(static_cast<int>(fps)), {10.0f, 10.0f});
        window_handler.displayText(font, "Objects: " + std::to_string(object_count), {10.0f, 40.0f});
        window_handler.display();

        rainbow_index = (rainbow_index + 1) % rainbow_count;
    }
    return 0;
}
