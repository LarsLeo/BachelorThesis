#include "MessageDelayGenerator.h"

float MessageDelayGenerator::delay = 0.02;
static std::geometric_distribution<int> distribution(0.2);
static std::default_random_engine generator;

float MessageDelayGenerator::getGeometricMessageDelay() {
    int number = distribution(generator);
    return number * delay;
}
