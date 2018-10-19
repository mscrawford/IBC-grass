#include <random>

#include "RandomGenerator.h"

int RandomGenerator::getUniformInt(int thru)
{
    return static_cast<int>( floor(get01() * thru) );
}

double RandomGenerator::get01()
{
    std::uniform_real_distribution<double> dist(0, 1);
    return dist(rng);
}

double RandomGenerator::getGaussian(double mean, double sd)
{
    std::normal_distribution<double> dist(mean, sd);
    return dist(rng);
}
