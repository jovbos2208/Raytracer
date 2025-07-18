#pragma once
#include "Vector3.h"
#include <string>
#include <cstring>

struct Ray {
    Vector3 origin;
    Vector3 direction;
    Vector3 momentum;
    Vector3 velocity;
    double energy = 0.0;
    double speciesMass = 0.0;
    double speciesDensity = 0.0;
    std::string speciesName;
    bool active = true;
    double weight = 1.0;
    int panelId = -1;
};

// 🧱 Packbar gemachtes Struct für MPI

struct RayMPI {
    double origin[3];
    double direction[3];
    double momentum[3];
    double velocity[3];
    double energy;
    double speciesMass;
    double speciesDensity;
    char speciesName[32];
    bool active;
    double weight;
    int panelId;
};


inline RayMPI toRayMPI(const Ray& ray) {
    RayMPI r{};
    for (int i = 0; i < 3; ++i) {
        r.origin[i] = ray.origin[i];
        r.direction[i] = ray.direction[i];
        r.momentum[i] = ray.momentum[i];
        r.velocity[i] = ray.velocity[i];
    }
    r.energy = ray.energy;
    r.speciesMass = ray.speciesMass;
    r.speciesDensity = ray.speciesDensity;
    std::strncpy(r.speciesName, ray.speciesName.c_str(), sizeof(r.speciesName) - 1);
    r.active = ray.active;
    r.weight = ray.weight;
    r.panelId = ray.panelId;
    return r;
}

inline Ray fromRayMPI(const RayMPI& r) {
    Ray ray;
    ray.origin = Vector3(r.origin[0], r.origin[1], r.origin[2]);
    ray.direction = Vector3(r.direction[0], r.direction[1], r.direction[2]);
    ray.momentum = Vector3(r.momentum[0], r.momentum[1], r.momentum[2]);
    ray.velocity = Vector3(r.velocity[0], r.velocity[1], r.velocity[2]);
    ray.energy = r.energy;
    ray.speciesMass = r.speciesMass;
    ray.speciesDensity = r.speciesDensity;
    ray.speciesName = std::string(r.speciesName);
    ray.active = r.active;
    ray.weight = r.weight;
    ray.panelId = r.panelId;
    return ray;
}
