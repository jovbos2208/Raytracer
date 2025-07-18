#include "ConfigLoader.h"
#include "ini.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

/// @brief Internal context structure used during INI parsing.
struct INIContext {
    SimulationConfig* config;
    std::string currentSection;
};

/// @brief Handler function for each key-value pair encountered by the INI parser.
static int iniHandler(void* user, const char* section, const char* name, const char* value) {
    INIContext* context = static_cast<INIContext*>(user);
    SimulationConfig* cfg = context->config;
    std::string key = name;
    std::string sectionStr = section;

    context->currentSection = sectionStr;

    // Read basic configuration options
    if (key == "geometryFile") {
        cfg->geometryFile = value;
    } else if (key == "rayCount") {
        cfg->rayCount = std::stoi(value);
    } else if (key == "reflectionRatio") {
        cfg->reflectionRatio = std::stod(value);
    } else if (key == "absorptionRatio") {
        cfg->absorptionRatio = std::stod(value);
    } else if (key == "energyLoss") {
        cfg->energyLoss = std::stod(value);
    } else if (key == "temperature") {
        cfg->temperature = std::stod(value);
    } else if (key == "mass_density") {
        cfg->massDensity = std::stod(value);
    } else if (key == "direction") {
        // Parse flow direction from comma-separated values
        std::stringstream ss(value);
        std::string component;
        std::vector<double> dirComponents;

        while (std::getline(ss, component, ',')) {
            dirComponents.push_back(std::stod(component));
        }

        if (dirComponents.size() == 3) {
            cfg->flowVelocity = {dirComponents[0], dirComponents[1], dirComponents[2]};
        }
    } else if (sectionStr.find("species:") == 0) {
        // Parse species-specific properties
        std::string speciesName = sectionStr.substr(8); // Skip "species:"
        SpeciesInfo& sp = cfg->species[speciesName];

        if (key == "density") {
            sp.density = std::stod(value);
        } else if (key == "mass") {
            sp.mass = std::stod(value);
        }
    }

    return 1; // Success
}

/// @brief Load configuration from an INI file.
/// @param filename The path to the config file.
/// @return True if successful, false on failure.
bool ConfigLoader::loadFromFile(const std::string& filename) {
    INIContext context;
    context.config = &config;

    if (ini_parse(filename.c_str(), iniHandler, &context) < 0) {
        std::cerr << "❌ Failed to load config file: " << filename << std::endl;
        return false;
    }

    return true;
}

/// @brief Get the parsed configuration object.
/// @return Parsed SimulationConfig instance.
SimulationConfig ConfigLoader::getConfig() const {
    return config;
}

