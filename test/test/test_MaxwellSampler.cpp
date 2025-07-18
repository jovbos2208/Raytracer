#include "MaxwellSampler.h"
#include "Vector3.h"
#include <iostream>
#include <cmath>
#include <cassert>

void test_maxwell_sampler_statistical_behavior() {
    const double T = 300.0;                  // Temperatur [K]
    const double m = 4.65e-26;               // Masse [kg]
    const Vector3 drift(100.0, 0.0, 0.0);    // Driftgeschwindigkeit in x

    MaxwellSampler sampler(T, m, drift);

    const int numSamples = 200000;
    Vector3 avgVel = {0.0, 0.0, 0.0};

    for (int i = 0; i < numSamples; ++i) {
        Vector3 v = sampler.sampleVelocity();
        avgVel += v;
    }

    avgVel = avgVel / static_cast<double>(numSamples);

    std::cout << "Gemittelte Geschwindigkeit: (" 
              << avgVel.x << ", " << avgVel.y << ", " << avgVel.z << ")\n";

    // Drift in x-Richtung sollte erhalten bleiben
    double relErrorX = std::abs(avgVel.x - drift.x) / drift.x;
    assert(relErrorX < 0.1); // < 10% Fehler erlaubt

    // Querkomponenten sollten ≈ 0 sein (symmetrisch verteilt)
    assert(std::abs(avgVel.y) < 10.0);  // Toleranz je nach stddev
    assert(std::abs(avgVel.z) < 10.0);

    std::cout << "[OK] test_maxwell_sampler_statistical_behavior ✅\n";
}

int main() {
    test_maxwell_sampler_statistical_behavior();
    return 0;
}

