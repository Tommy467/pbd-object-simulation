cmake_minimum_required(VERSION 3.12)
project(PBD LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
set(SFML_DIR "D:/Workspace/Environment/SFML-vc17-2.6.2/lib/cmake/SFML")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

find_package(SFML REQUIRED COMPONENTS audio network graphics window system)
find_package(OpenMP REQUIRED)
find_package(CUDAToolkit REQUIRED)

include_directories(${SFML_INCLUDE_DIR})

set_source_files_properties(src/particle_cuda.cu PROPERTIES LANGUAGE CUDA)

file(GLOB SRC_FILES
        "${SRC_DIR}/*.c"
        "${SRC_DIR}/*.cpp"
        "${SRC_DIR}/*.hpp"
        "${SRC_DIR}/*.cu")

add_executable(${PROJECT_NAME} ${SRC_FILES})

if (SFML_FOUND)
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-network sfml-graphics sfml-window sfml-system)
endif()
if (OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
endif()
if (CUDAToolkit_FOUND)
    target_link_libraries(${PROJECT_NAME} CUDA::cudart CUDA::cublas CUDA::cufft CUDA::curand CUDA::cusolver CUDA::cusparse)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

