#include "MaxwellSampler.h"
#include <cmath>

// Boltzmann constant in J/K
constexpr double kB = 1.380649e-23;

/**
 * @brief Constructor for the MaxwellSampler.
 * 
 * Initializes the sampler with a given temperature, particle mass, and drift velocity.
 * It precomputes the standard deviation for the Maxwell-Boltzmann distribution and
 * prepares random number generators for sampling.
 * 
 * @param temperature Temperature in Kelvin.
 * @param mass Particle mass in kilograms.
 * @param drift Drift velocity vector (bulk flow).
 */
MaxwellSampler::MaxwellSampler(double temperature, double mass, Vector3 drift)
    : rng(std::random_device{}()),
      normal(0.0, std::sqrt(kB * temperature / mass)),  // Normal distribution for speed
      uniDist(0.0, 1.0),                                // Uniform distribution for angles
      mass(mass),
      temperature(temperature),
      driftVelocity(drift),
      nFlux(drift.normalize())                          // Normalized flow direction
{
    stddev = std::sqrt(kB * temperature / mass);
}

/**
 * @brief Samples a velocity vector from the Maxwell-Boltzmann distribution.
 * 
 * The sample is aligned to the drift (flow) direction, and only forward-moving
 * particles (i.e., with a velocity component in the drift direction) are allowed.
 * 
 * The sampling uses:
 * - Cosine-weighted hemisphere sampling for direction
 * - Normal distribution for speed magnitude
 * 
 * @return A sampled velocity vector in 3D space.
 */
Vector3 MaxwellSampler::sampleVelocity() const {
    Vector3 v;
    do {
        // Sample direction in hemisphere using cosine-weighted distribution
        double r1 = uniDist(rng);
        double r2 = uniDist(rng);
        double phi = 2.0 * M_PI * r2;
        double cosTheta = std::sqrt(r1);
        double sinTheta = std::sqrt(1.0 - r1);

        // Construct orthonormal basis around nFlux (drift direction)
        Vector3 z = nFlux;
        Vector3 x = (std::abs(z.x) > 0.9 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).cross(z).normalize();
        Vector3 y = z.cross(x);

        // Compose final direction vector
        Vector3 dir = (x * std::cos(phi) * sinTheta +
                       y * std::sin(phi) * sinTheta +
                       z * cosTheta).normalize();

        // Sample speed from 1D Maxwellian (absolute value of normal distribution)
        double speed = std::abs(normal(rng));

        // Combine direction and speed, add drift velocity
        v = dir * speed + driftVelocity;

    } while (v.dot(nFlux) <= 0);  // Accept only forward-moving particles

    return v;
}

