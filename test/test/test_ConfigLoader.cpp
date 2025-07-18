#include "ConfigLoader.h"
#include <cassert>
#include <fstream>
#include <iostream>

// Hilfsfunktion zum Erstellen einer temporären INI-Datei
std::string writeTestINI(const std::string& content) {
    std::string filename = "test_config.ini";
    std::ofstream out(filename);
    out << content;
    out.close();
    return filename;
}

void test_config_loader_parses_correctly() {
    std::string iniContent = R"ini(
[general]
geometry = geom.obj
heatmap = heat.csv
ray_count = 500
max_bounces = 3
reflection_ratio = 0.8
absorption_ratio = 0.2
energy_loss = 0.05
specularFraction = 0.4
model = DRIA
alpha_E = 0.9
T_w = 320.0

[flow]
temperature = 280.0
velocity = 500.0
direction = 1,0,0

[species:N2]
density = 1.0
mass = 4.65e-26
)ini";

    std::string filename = writeTestINI(iniContent);

    ConfigLoader loader;
    bool success = loader.loadFromFile(filename);
    assert(success);

    const auto& cfg = loader.getConfig();

    assert(cfg.geometryFile == "geom.obj");
    assert(cfg.heatmapOutputFile == "heat.csv");
    assert(cfg.rayCount == 500);
    assert(cfg.maxBounces == 3);
    assert(std::abs(cfg.reflectionRatio - 0.8) < 1e-6);
    assert(std::abs(cfg.absorptionRatio - 0.2) < 1e-6);
    assert(std::abs(cfg.energyLoss - 0.05) < 1e-6);
    assert(cfg.model == "DRIA");
    assert(std::abs(cfg.energyAccommodation - 0.9) < 1e-6);
    assert(std::abs(cfg.WallTemp - 320.0) < 1e-6);

    // Strömung
    assert(std::abs(cfg.temperature - 280.0) < 1e-6);
    assert(std::abs(cfg.flowVelocity.x - 500.0) < 1e-6);
    assert(std::abs(cfg.flowVelocity.y) < 1e-6);
    assert(std::abs(cfg.flowVelocity.z) < 1e-6);

    // Spezies
    assert(cfg.species.count("n2") == 1);
    assert(std::abs(cfg.species.at("n2").density - 1.0) < 1e-6);
    assert(std::abs(cfg.species.at("n2").mass - 4.65e-26) < 1e-30);

    std::cout << "[OK] test_config_loader_parses_correctly\n";
}

int main() {
    test_config_loader_parses_correctly();
    return 0;
}
