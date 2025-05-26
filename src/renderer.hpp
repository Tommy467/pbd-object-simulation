#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>

#include "utils.hpp"
#include "physics_handler.hpp"
#include "window_handler.hpp"

class Renderer {
public:
    explicit Renderer(PhysicsHandler &_physics_handler)
        : physics_handler(_physics_handler)
        , world_va(sf::Quads, 4)
        , objects_va(sf::Quads)
    {
        initializeWorldVA();

        object_texture.loadFromFile("D:/Workspace/C++/PBD/res/circle.png");
        object_texture.generateMipmap();
        object_texture.setSmooth(true);
    }

    void render(WindowHandler &window_handler) {
        window_handler.draw(world_va);

        sf::RenderStates states;
        states.texture = &object_texture;
        window_handler.draw(world_va, states);

        updateParticlesVA();
        window_handler.draw(objects_va, states);
    }

    static void HSVtoRGB(const float h, const float s, const float v, float &r, float &g, float &b) {
        const int i = static_cast<int>(h * 6);
        const float f = h * 6 - static_cast<float>(i);
        const float p = v * (1 - s);
        const float q = v * (1 - f * s);
        const float t = v * (1 - (1 - f) * s);
        switch (i % 6) {
            case 0: r = v, g = t, b = p; break;
            case 1: r = q, g = v, b = p; break;
            case 2: r = p, g = v, b = t; break;
            case 3: r = p, g = q, b = v; break;
            case 4: r = t, g = p, b = v; break;
            default: r = v, g = p, b = q; break;
        }
        r *= 255;
        g *= 255;
        b *= 255;
    }

private:
    void initializeWorldVA() {
        world_va[0].position = {0.0f, 0.0f};
        world_va[1].position = {physics_handler.getWorldSize().x, 0.0f};
        world_va[2].position = {physics_handler.getWorldSize().x, physics_handler.getWorldSize().y};
        world_va[3].position = {0.0f, physics_handler.getWorldSize().y};

        const auto bg_color = sf::Color(50, 50, 50);
        world_va[0].color = bg_color;
        world_va[1].color = bg_color;
        world_va[2].color = bg_color;
        world_va[3].color = bg_color;
    }

    void updateParticlesVA() {

        constexpr float texture_size = 1024.0f;

    #ifdef USE_CPU
        objects_va.resize(physics_handler.getObjectsCount() * 4);
        #pragma omp parallel for num_threads(omp_get_max_threads())
        for (int32_t i = 0; i < physics_handler.getObjectsCount(); ++i) {
            const uint32_t idx = i << 2;

            const Object &object = physics_handler.getObjectAoSAt(i);
            objects_va[idx + 0].position = V2f{ object.position_x - object.radius, object.position_y - object.radius };
            objects_va[idx + 1].position = V2f{ object.position_x + object.radius, object.position_y - object.radius };
            objects_va[idx + 2].position = V2f{ object.position_x + object.radius, object.position_y + object.radius };
            objects_va[idx + 3].position = V2f{ object.position_x - object.radius, object.position_y + object.radius };
            const sf::Color color = { static_cast<sf::Uint8>(object.color_r), static_cast<sf::Uint8>(object.color_g), static_cast<sf::Uint8>(object.color_b)};
            objects_va[idx + 0].color = color;
            objects_va[idx + 1].color = color;
            objects_va[idx + 2].color = color;
            objects_va[idx + 3].color = color;
            objects_va[idx + 0].texCoords = {0.0f, 0.0f};
            objects_va[idx + 1].texCoords = {texture_size, 0.0f};
            objects_va[idx + 2].texCoords = {texture_size, texture_size};
            objects_va[idx + 3].texCoords = {0.0f, texture_size};
        }
    #elif defined USE_GPU
        objects_va.resize(physics_handler.getObjectsCount() * 4);
        const Object *objects = physics_handler.getObjects();
        #pragma omp parallel for num_threads(omp_get_max_threads())
        for (int32_t i = 0; i < physics_handler.getObjectsCount(); ++i) {
            const uint32_t idx = i << 2;

            objects_va[idx + 0].position = V2f{ objects->position_x[i] - objects->radius[i], objects->position_y[i] - objects->radius[i] };
            objects_va[idx + 1].position = V2f{ objects->position_x[i] + objects->radius[i], objects->position_y[i] - objects->radius[i] };
            objects_va[idx + 2].position = V2f{ objects->position_x[i] + objects->radius[i], objects->position_y[i] + objects->radius[i] };
            objects_va[idx + 3].position = V2f{ objects->position_x[i] - objects->radius[i], objects->position_y[i] + objects->radius[i] };
            const sf::Color color = { static_cast<sf::Uint8>(objects->color_r[i]), static_cast<sf::Uint8>(objects->color_g[i]), static_cast<sf::Uint8>(objects->color_b[i])};
            objects_va[idx + 0].color = color;
            objects_va[idx + 1].color = color;
            objects_va[idx + 2].color = color;
            objects_va[idx + 3].color = color;
            objects_va[idx + 0].texCoords = {0.0f, 0.0f};
            objects_va[idx + 1].texCoords = {texture_size, 0.0f};
            objects_va[idx + 2].texCoords = {texture_size, texture_size};
            objects_va[idx + 3].texCoords = {0.0f, texture_size};
        }
    #endif

    }


    PhysicsHandler  &physics_handler;
    sf::VertexArray world_va;
    sf::VertexArray objects_va;
    sf::Texture     object_texture;
};

#endif
