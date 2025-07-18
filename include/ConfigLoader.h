#pragma once
#include "INIReader.h"
#include <string>
#include <iostream>
#include "Vector3.h"

struct SpeciesInfo {
    double density;
    double mass;
};


struct SimulationConfig {
    std::string geometryFile = "models/Cube.obj";
    std::string heatmapOutputFile = "output.vtk";
    std::string model = "DRIA";
    int rayCount = 1000;
    int maxBounces = 5;

    double reflectionRatio = 0.5;
    double absorptionRatio = 0.2;
    double energyLoss = 0.1;
    double energyAccommodation = 1.0;
    double kB = 1.380649e-23;
    double WallTemp = 300.0;
    double specularFraction = 0.3;
    double mass_density = 1e-10;
    
    double temperature = 300.0;
    Vector3 flowVelocity = Vector3{0.0, 0.0, -1.0};
    std::map<std::string, SpeciesInfo> species;
};
class ConfigLoader {
public:
    bool loadFromFile(const std::string& filename);
    const SimulationConfig& getConfig() const;

private:
    SimulationConfig config;
};


