#pragma once
#include "Vector3.h"
#include <random>

class MaxwellSampler {
public:
    MaxwellSampler(double temperature, double mass, Vector3 drift);

    // Eine einzelne Geschwindigkeit im Halbraum (gerichtet entlang Drift)
    Vector3 sampleVelocity();

private:
    double mass;
    double temperature;
    double stddev;

    Vector3 driftVelocity;  // Strömungsgeschwindigkeit
    Vector3 nFlux;          // Normalisierte Strömungsrichtung

    mutable std::mt19937 rng;
    mutable std::normal_distribution<double> normal;
    mutable std::uniform_real_distribution<double> uniDist;
};

