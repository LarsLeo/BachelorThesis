#ifndef ENTITIES_UTILITY_MESSAGEDELAYGENERATOR_H_
#define ENTITIES_UTILITY_MESSAGEDELAYGENERATOR_H_

#include <random>

class MessageDelayGenerator {
public:
        static float getGeometricMessageDelay();

private:
        static float delay; // The added delay per distribution tick in seconds.
};

#endif
