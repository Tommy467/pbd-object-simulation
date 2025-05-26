#ifndef RANDOM_NUMBER_GENERATOR_HPP
#define RANDOM_NUMBER_GENERATOR_HPP

#include <random>
#include <ctime>

class RandomNumberGenerator {
public:
    explicit RandomNumberGenerator(const time_t time = std::time(nullptr))
    {
        gen.seed(static_cast<unsigned int>(time));
    }

    ~RandomNumberGenerator() = default;

    static int getInt(const int min, const int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }

    static float getFloat(const float min, const float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

    static double getDouble(const double min, const double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(gen);
    }

private:
    static std::mt19937 gen;
};

std::mt19937 RandomNumberGenerator::gen;

#endif
