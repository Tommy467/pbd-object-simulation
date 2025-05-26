#ifndef GRID_HELPER_HPP
#define GRID_HELPER_HPP

#include "utils.hpp"

#ifdef USE_CPU

#include <cstdint>
#include <vector>

#include "object.hpp"

struct Grid {
    explicit Grid(const int32_t _index)
        : index(_index)
    {}

    void clear() {
        objects_idx.clear();
    }

    void addObject(const int32_t idx) {
        objects_idx.push_back(idx);
    }

    int32_t width = 1;
    int32_t height = 1;
    int32_t index;
    std::vector<int32_t> objects_idx;
};

class GridHelper {
public:
    GridHelper(const int32_t _world_width, const int32_t _world_height)
        : world_width(_world_width)
        , world_height(_world_height)
    {
        grids_count = world_width * world_height;
        for (int32_t i = 0; i < grids_count; ++i) {
            grids.emplace_back(i);
        }
    }

    Grid &getGridAt(const int32_t index) {
        return grids[index];
    }

    [[nodiscard]]
    int32_t getGridsCount() const {
        return grids_count;
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
    int32_t getObjectIndexInGrid(const Object &object) const {
        const float pos_x = object.position_x;
        const float pos_y = object.position_y;
        const auto x = static_cast<int32_t>(floorf(pos_x));
        const auto y = static_cast<int32_t>(floorf(pos_y));
        return x + y * getGridsWidthCount();
    }

    void updateGrids(const std::vector<Object> &objects) {
        for (auto &grid : grids) {
            grid.clear();
        }

        for (int32_t idx = 0; idx < objects.size(); idx++) {
            const int32_t grid_index = getObjectIndexInGrid(objects[idx]);
            grids[grid_index].addObject(idx);
        }
    }

private:
    int32_t world_width;
    int32_t world_height;
    std::vector<Grid> grids;
    int32_t grids_count;
};

#endif

#endif
