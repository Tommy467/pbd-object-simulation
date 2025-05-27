#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cassert>
// #include <__msvc_ostream.hpp>
#include <iostream>
#include <fstream>

#include "utils.hpp"
#include "object.hpp"

#ifdef USE_GPU

#ifdef OUTPUT_RESULTS
extern std::ofstream output_file;
#endif
extern int32_t particle_min_count;

constexpr int num_cell = 8;
struct Grids_Cuda {
    int32_t *grid_index;
    int32_t *object_index;
    int32_t *object_counts;
    int32_t grid_count;
} grids;

void Object_initDeviceMemory(Object *objects) {
    if (objects->device_allocated) return;
    cudaMalloc(&objects->d_position_x,      sizeof(float) * N);
    cudaMalloc(&objects->d_position_y,      sizeof(float) * N);
    cudaMalloc(&objects->d_last_position_x, sizeof(float) * N);
    cudaMalloc(&objects->d_last_position_y, sizeof(float) * N);
    cudaMalloc(&objects->d_radius,          sizeof(float) * N);
    objects->device_allocated = true;
}

void Object_freeDeviceMemory(Object *objects) {
    if (!objects->device_allocated) return;
    cudaFree(objects->d_position_x);
    cudaFree(objects->d_position_y);
    cudaFree(objects->d_last_position_x);
    cudaFree(objects->d_last_position_y);
    cudaFree(objects->d_radius);
    objects->device_allocated = false;
}

void Grids_initDeviceMemory(const int32_t world_width, const int32_t world_height) {
    const int32_t size = world_width * world_height;
    cudaMalloc(&grids.grid_index, sizeof(int32_t) * size);
    cudaMalloc(&grids.object_index, sizeof(int32_t) * size * num_cell);
    cudaMalloc(&grids.object_counts, sizeof(int32_t) * size);
    grids.grid_count = size;
}

void Grids_freeDeviceMemory() {
    cudaFree(grids.grid_index);
    cudaFree(grids.object_index);
    cudaFree(grids.object_counts);
}

void objectCopyToDevice(const Object *objects) {
    if (!objects->device_allocated) return;
    cudaMemcpy(objects->d_position_x,      objects->position_x,      sizeof(float) * objects->size, cudaMemcpyHostToDevice);
    cudaMemcpy(objects->d_position_y,      objects->position_y,      sizeof(float) * objects->size, cudaMemcpyHostToDevice);
    cudaMemcpy(objects->d_last_position_x, objects->last_position_x, sizeof(float) * objects->size, cudaMemcpyHostToDevice);
    cudaMemcpy(objects->d_last_position_y, objects->last_position_y, sizeof(float) * objects->size, cudaMemcpyHostToDevice);
    cudaMemcpy(objects->d_radius,          objects->radius,          sizeof(float) * objects->size, cudaMemcpyHostToDevice);
}

void objectCopyToHost(Object *objects) {
    if (!objects->device_allocated) return;
    cudaMemcpy(objects->position_x,      objects->d_position_x,      sizeof(float) * objects->size, cudaMemcpyDeviceToHost);
    cudaMemcpy(objects->position_y,      objects->d_position_y,      sizeof(float) * objects->size, cudaMemcpyDeviceToHost);
    cudaMemcpy(objects->last_position_x, objects->d_last_position_x, sizeof(float) * objects->size, cudaMemcpyDeviceToHost);
    cudaMemcpy(objects->last_position_y, objects->d_last_position_y, sizeof(float) * objects->size, cudaMemcpyDeviceToHost);
}

__global__ void updateObjects_kernel(
    float *position_x, float *position_y,
    float *last_position_x, float *last_position_y,
    const float acceleration_x, const float acceleration_y,
    const float *radius,
    const int size, const float delta_time, const float world_size_x, const float world_size_y
) {
    unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= size) return;
    const float last_movement_x = position_x[idx] - last_position_x[idx];
    const float last_movement_y = position_y[idx] - last_position_y[idx];
    constexpr float velocity_damping = 0.001f;
    float new_position_x = position_x[idx] + last_movement_x + (acceleration_x - last_movement_x * velocity_damping) * (delta_time * delta_time);
    float new_position_y = position_y[idx] + last_movement_y + (acceleration_y - last_movement_y * velocity_damping) * (delta_time * delta_time);

    constexpr float margin = 2.0f;
    if (new_position_x < margin + radius[idx])                     { new_position_x = margin + radius[idx]; }
    else if (new_position_x > world_size_x - margin - radius[idx]) { new_position_x = world_size_x - margin - radius[idx]; }
    if (new_position_y < margin + radius[idx])                     { new_position_y = margin + radius[idx]; }
    else if (new_position_y > world_size_y - margin - radius[idx]) { new_position_y = world_size_y - margin - radius[idx]; }

    last_position_x[idx] = position_x[idx];
    last_position_y[idx] = position_y[idx];
    position_x[idx] = new_position_x;
    position_y[idx] = new_position_y;
}

__global__ void initGridCounts_kernel(int32_t *object_counts, const int grid_count) {
    const unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < grid_count) {
        object_counts[idx] = 0;
    }
}

__global__ void assignObjectsToGrid_kernel(
    const float *position_x, const float *position_y,
    const int object_count,
    const int world_width,
    const int world_height,
    int32_t *object_index,
    int32_t *object_counts
) {
    const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= object_count) return;
    const int grid_x = static_cast<int>(floorf(position_x[i]));
    const int grid_y = static_cast<int>(floorf(position_y[i]));
    if (grid_x < 0 || grid_x >= world_width || grid_y < 0 || grid_y >= world_height) return;
    // const int target_idx = grid_y * world_width + grid_x;
    const int target_idx = grid_x * world_height + grid_y;
    const int offset = atomicAdd(&object_counts[target_idx], 1);
    if (offset < num_cell) {
        object_index[target_idx * num_cell + offset] = static_cast<int>(i);
    }
}

__global__ void solveCollisions_kernel(
    float *position_x, float *position_y, const float *radius,
    const int32_t *object_index, const int32_t *object_counts,
    const int grid_count, const int world_width, const int world_height
) {
    const int grid_idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (grid_idx >= grid_count) return;
    // const int grid_x = grid_idx % world_width;
    // const int grid_y = grid_idx / world_width;
    const int grid_x = grid_idx / world_height;
    const int grid_y = grid_idx % world_height;
    const int count1 = object_counts[grid_idx];
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            const int nx = grid_x + dx;
            const int ny = grid_y + dy;
            if (nx < 0 || nx >= world_width || ny < 0 || ny >= world_height) continue;
            // int nidx = ny * world_width + nx;
            int nidx = nx * world_height + ny;
            int count2 = object_counts[nidx];
            for (int i = 0; i < count1; ++i) {
                int obj1 = object_index[grid_idx * num_cell + i];
                for (int j = 0; j < count2; ++j) {
                    int obj2 = object_index[nidx * num_cell + j];
                    if (obj1 >= 0 && obj2 >= 0 && obj1 != obj2) {
                        float dx = position_x[obj1] - position_x[obj2];
                        float dy = position_y[obj1] - position_y[obj2];
                        float dist2 = dx * dx + dy * dy;
                        float r_sum = radius[obj1] + radius[obj2];
                        if (dist2 < r_sum * r_sum && dist2 > 1e-6f) {
                            float dist = sqrtf(dist2);
                            float delta = 0.5f * (r_sum - dist);
                            float nx = dx / dist;
                            float ny = dy / dist;
                            atomicAdd(&position_x[obj1], nx * delta);
                            atomicAdd(&position_y[obj1], ny * delta);
                            atomicAdd(&position_x[obj2], -nx * delta);
                            atomicAdd(&position_y[obj2], -ny * delta);
                        }
                    }
                }
            }
        }
    }
}

void solveCollisions(const Object *objects, const int32_t *object_index, const int32_t *object_counts, const int grid_count, const int world_width, const int world_height) {
    // int blockSize = 128;
    int gridSize = (grid_count + gpu_block_size - 1) / gpu_block_size;
    solveCollisions_kernel<<<gridSize, gpu_block_size>>>(
        objects->d_position_x, objects->d_position_y, objects->d_radius,
        object_index, object_counts,
        grid_count, world_width, world_height
    );
    // cudaDeviceSynchronize();
}

void updateObjects(const Object *objects, const float delta_time, const float world_size_x, const float world_size_y) {
    const int size = objects->size;
    // int blockSize = 256;
    int gridSize = (size + gpu_block_size - 1) / gpu_block_size;
    updateObjects_kernel<<<gridSize, gpu_block_size>>>(
        objects->d_position_x, objects->d_position_y,
        objects->d_last_position_x, objects->d_last_position_y,
        objects->acceleration_x, objects->acceleration_y,
        objects->d_radius,
        size, delta_time, world_size_x, world_size_y
    );
    // cudaDeviceSynchronize();
}

void updateGrids(const Object *objects, const int world_width, const int world_height) {
    const int grid_count = grids.grid_count;
    // int blockSize = 256;
    int gridSize = (grid_count + gpu_block_size - 1) / gpu_block_size;
    initGridCounts_kernel<<<gridSize, gpu_block_size>>>(grids.object_counts, grid_count);
    // cudaDeviceSynchronize();

    int object_count = objects->size;
    gridSize = (object_count + gpu_block_size - 1) / gpu_block_size;
    assignObjectsToGrid_kernel<<<gridSize, gpu_block_size>>>(
        objects->d_position_x, objects->d_position_y,
        object_count,
        world_width,
        world_height,
        grids.object_index,
        grids.object_counts
    );
    // cudaDeviceSynchronize();
}

extern void updatePhysics(Object *objects, const float sub_delta_time, const float sub_steps, const float world_size_x, const float world_size_y) {
    if (objects->size < 0) return;
    objectCopyToDevice(objects);

#ifdef OUTPUT_RESULTS
    cudaEvent_t start, end;
    float elapsedTime = 0.0f;

    cudaEventCreate(&start);
    cudaEventCreate(&end);

    cudaEventRecord(start, nullptr);
#endif

    for (int i = 0; i < static_cast<int>(sub_steps); ++i) {
        updateObjects(objects, sub_delta_time, world_size_x, world_size_y);
        updateGrids(objects, static_cast<int>(world_size_x), static_cast<int>(world_size_y));
        solveCollisions(objects, grids.object_index, grids.object_counts, grids.grid_count, static_cast<int>(world_size_x), static_cast<int>(world_size_y));
    }

#ifdef OUTPUT_RESULTS
    cudaEventRecord(end, nullptr);
    cudaEventSynchronize(end);

    cudaEventElapsedTime(&elapsedTime, start, end);
    if (objects->size > particle_min_count) {
        output_file << elapsedTime * 1000 << ",";
    }

    cudaEventDestroy(start);
    cudaEventDestroy(end);
#endif

    objectCopyToHost(objects);
}

#endif
