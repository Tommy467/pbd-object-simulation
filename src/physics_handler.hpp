#ifndef PHYSICS_HANDLER_HPP
#define PHYSICS_HANDLER_HPP

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <omp.h>

#include "grid_helper.hpp"
#include "object.hpp"
#include "utils.hpp"

#ifdef USE_GPU
extern void Object_initDeviceMemory(Object *objects);
extern void Object_freeDeviceMemory(Object *objects);
extern void Grids_initDeviceMemory(int32_t world_width, int32_t world_height);
extern void Grids_freeDeviceMemory();
extern void updatePhysics(Object *objects, float sub_delta_time, float sub_steps, float world_size_x, float world_size_y);
#endif

class PhysicsHandler {
public:
    explicit PhysicsHandler(const V2f size)
        : world_size(size)
    #ifdef USE_CPU
        , grid_helper(static_cast<int32_t>(size.x), static_cast<int32_t>(size.y))
    #endif
    {
    #ifdef USE_GPU
        objects = new Object();
        Object_initDeviceMemory(objects);
        Grids_initDeviceMemory(static_cast<int32_t>(size.x), static_cast<int32_t>(size.y));
    #endif
    }

    #ifdef USE_GPU
    ~PhysicsHandler() {
        Object_freeDeviceMemory(objects);
        Grids_freeDeviceMemory();
    }
    #endif

    [[nodiscard]]
    int32_t createObject(const float pos_x, const float pos_y, const float vel_x = 0.0f, const float vel_y = 0.0f, const float radius = 0.5f, const float color_r = 255.0f, const float color_g = 255.0f, const float color_b = 255.0f) {
    #ifdef USE_CPU
        objects.emplace_back(pos_x, pos_y, vel_x, vel_y, radius, color_r, color_g, color_b);
        return static_cast<int32_t>(objects.size()) - 1;
    #elif defined USE_GPU
        objects->position_x[objects->size] = pos_x;
        objects->position_y[objects->size] = pos_y;
        objects->last_position_x[objects->size] = pos_x - vel_x;
        objects->last_position_y[objects->size] = pos_y - vel_y;
        objects->radius[objects->size] = radius;
        objects->color_r[objects->size] = color_r;
        objects->color_g[objects->size] = color_g;
        objects->color_b[objects->size] = color_b;
        return objects->size++;
    #endif
    }

    [[nodiscard]]
    V2f getWorldSize() const {
        return world_size;
    }

    [[nodiscard]]
    int32_t getObjectsCount() const {
    #ifdef USE_CPU
        return static_cast<int32_t>(objects.size());
    #elif defined USE_GPU
        return objects->size;
    #endif
    }

    #ifdef USE_CPU
    Object &getObjectAoSAt(const int32_t idx) {
        return objects[idx];
    }
    #endif

    #ifdef USE_GPU
    [[nodiscard]]
    Object *getObjects() const {
        return objects;
    }
    #endif

    void update(const float delta_time) {
        constexpr float sub_steps = 8.0f;
        const float sub_delta_time = delta_time / sub_steps;
        #ifdef USE_CPU
        for (int32_t i = 0; i < static_cast<int32_t>(sub_steps); ++i) {
            updateGrids();
            solveCollisions();
            updateObjects(sub_delta_time);
        }
        #elif defined USE_GPU
            updatePhysics(objects, sub_delta_time, sub_steps, world_size.x, world_size.y);
        #endif
    }


private:
    #ifdef USE_CPU
    void solveCollisions() {
        #pragma omp parallel for num_threads(omp_get_max_threads()) default(none)
        for (int32_t idx = 0; idx < grid_helper.getGridsCount(); ++idx) {
            if (grid_helper.getGridAt(idx).objects_idx.size() <= 0) continue;
            checkGridCollisions(idx, idx - 1);
            checkGridCollisions(idx, idx);
            checkGridCollisions(idx, idx + 1);
            checkGridCollisions(idx, idx - grid_helper.getGridsWidthCount() - 1);
            checkGridCollisions(idx, idx - grid_helper.getGridsWidthCount());
            checkGridCollisions(idx, idx - grid_helper.getGridsWidthCount() + 1);
            checkGridCollisions(idx, idx + grid_helper.getGridsWidthCount() - 1);
            checkGridCollisions(idx, idx + grid_helper.getGridsWidthCount());
            checkGridCollisions(idx, idx + grid_helper.getGridsWidthCount() + 1);
        }
    }

    void checkGridCollisions(const int32_t grid1_idx, const int32_t grid2_idx) {
        if (grid2_idx < 0 || grid2_idx >= grid_helper.getGridsCount()) {
            return;
        }

        const Grid &grid1 = grid_helper.getGridAt(grid1_idx);
        const Grid &grid2 = grid_helper.getGridAt(grid2_idx);

        for (const int i : grid1.objects_idx) {
            for (const int j : grid2.objects_idx) {
                solveContact(i, j);
            }
        }
    }

    void solveContact(const int32_t obj_idx1, const int32_t obj_idx2) {
        const float delta_x = objects[obj_idx1].position_x - objects[obj_idx2].position_x;
        const float delta_y = objects[obj_idx1].position_y - objects[obj_idx2].position_y;
        const float dist2 = delta_x * delta_x + delta_y * delta_y;
        if (dist2 < objects[obj_idx1].radius + objects[obj_idx2].radius && dist2 > 1e-3f) {
            constexpr float response_coef = 1.0f;
            const float dist = sqrt(dist2);
            const float delta_dist = response_coef * 0.5f * (objects[obj_idx1].radius + objects[obj_idx2].radius - dist);
            const float col_vec_x = (delta_x / dist) * delta_dist;
            const float col_vec_y = (delta_y / dist) * delta_dist;
            objects[obj_idx1].position_x += col_vec_x;
            objects[obj_idx1].position_y += col_vec_y;
            objects[obj_idx2].position_x -= col_vec_x;
            objects[obj_idx2].position_y -= col_vec_y;
        }
    }

    void updateObjects(const float delta_time) {
        #pragma omp parallel for num_threads(omp_get_max_threads())
        for (int idx = 0; idx < objects.size(); ++idx) {
            const float last_movement_x = objects[idx].position_x - objects[idx].last_position_x;
            const float last_movement_y = objects[idx].position_y - objects[idx].last_position_y;
            constexpr float velocity_damping = 0.001f;
            float new_position_x = objects[idx].position_x + last_movement_x + (objects[idx].acceleration_x - last_movement_x * velocity_damping) * (delta_time * delta_time);
            float new_position_y = objects[idx].position_y + last_movement_y + (objects[idx].acceleration_y - last_movement_y * velocity_damping) * (delta_time * delta_time);

            constexpr float margin = 2.0f;
            if (new_position_x < margin)                     { new_position_x = margin; }
            else if (new_position_x > world_size.x - margin) { new_position_x = world_size.x - margin; }
            if (new_position_y < margin)                     { new_position_y = margin; }
            else if (new_position_y > world_size.y - margin) { new_position_y = world_size.y - margin; }

            objects[idx].last_position_x = objects[idx].position_x;
            objects[idx].last_position_y = objects[idx].position_y;
            objects[idx].position_x      = new_position_x;
            objects[idx].position_y      = new_position_y;
        }
    }

    void updateGrids() {
        grid_helper.updateGrids(objects);
    }
    #endif


    V2f world_size;
    #ifdef USE_CPU
    GridHelper grid_helper;
    std::vector<Object> objects;
    #elif defined USE_GPU
    Object *objects = nullptr;
    #endif
};

#endif
