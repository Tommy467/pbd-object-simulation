#ifndef GRID_HELPER_HPP
#define GRID_HELPER_HPP

#include "utils.hpp"

#ifdef USE_CPU

#include <cstdint>
#include <vector>

#include "object.hpp"

constexpr int num_cell = 16;

struct Grid {
    Grid() = default;

    void clear() {
        object_count = 0;
    }

    void addObject(const int32_t idx) {
        object_idx[object_count++] = idx;
    }

    int32_t object_idx[num_cell]{};
    int32_t object_count = 0;
};

class GridHelper {
public:
    GridHelper(const int32_t _world_width, const int32_t _world_height)
        : world_width(_world_width)
        , world_height(_world_height)
    {
        const int32_t grids_count = world_width * world_height;
        grids.resize(grids_count);
    }

    Grid &getGridAt(const int32_t index) {
        return grids[index];
    }

    [[nodiscard]]
    int32_t getGridsCount() const {
        return static_cast<int32_t>(grids.size());
    }

    [[nodiscard]]
    int32_t getGridsWidthCount() const {
        return world_width;
    }

    [[nodiscard]]
    int32_t getGridsHeightCount() const {
        return world_height;
    }

    [[nodiscard]]
    int32_t getGridIndexForObject(const Object &object) const {
        const auto idx_x = static_cast<int32_t>(floorf(object.position_x));
        const auto idx_y = static_cast<int32_t>(floorf(object.position_y));
        return idx_x * getGridsHeightCount() + idx_y;
    }

    void updateGrids(const std::vector<Object> &objects) {
        for (auto &grid : grids) {
            grid.clear();
        }

        for (int32_t idx = 0; idx < objects.size(); idx++) {
            const int32_t grid_index = getGridIndexForObject(objects[idx]);
            grids[grid_index].addObject(idx);
        }
    }

private:
    int32_t world_width, world_height;
    std::vector<Grid> grids;
};

#endif

#endif
