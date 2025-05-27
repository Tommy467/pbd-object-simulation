#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <functional>
#include <unordered_map>
#include <omp.h>

#define USE_CPU
// #define USE_GPU
// #define OUTPUT_RESULTS

using V2f = sf::Vector2f;
using V2i = sf::Vector2i;

using EventCallback = std::function<void(const sf::Event &event)>;
template<typename T>
using EventCallbackMap = std::unordered_map<T, EventCallback>;

const int cpu_threads = std::min(16, omp_get_max_threads());


#endif
