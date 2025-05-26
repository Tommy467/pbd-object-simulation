# Build Instructions

**SFML**, **CMAKE**, and **Nvidia Toolkit** needed to be installed.

The SFML version used in this project is 2.6.x.
For Linux, The SFML library can be installed using the following cmake command:
```cmake
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)
```
For Windows, you can download the SFML library from the official website and set it up in your CMakeLists.txt file, and you can use the following CMake command to include SFML:
```cmake
set(SFML_DIR "C:/path/to/SFML/lib/cmake/SFML")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
```
also, you need to move all the SFML dll files to the same directory as the executable file.

The Cmake version should be at least 3.12.0, otherwise, you should change "CUDAToolkit" to "CUDA" in the CMakeLists.txt file.
