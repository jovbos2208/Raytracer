#include "SimulationController.h"
#include "IntersectionEngine.h"
#include "ConfigLoader.h"
#include "Vector3.h"
#include "Triangle.h"
#include <iostream>
#include <cassert>

void test_simulation_controller_generates_shell_rays() {
    std::cout << "[TEST] SimulationController: generateMixedRays() mit BoundingBox-Shell\n";

    SimulationController controller;
    IntersectionEngine intersection;
    controller.setIntersectionEngine(&intersection);

    std::string meshPath = "models/Cube.obj";
    std::cout << "→ Lade Mesh: " << meshPath << "\n";
    controller.loadMesh(meshPath);

    const auto& mesh = controller.getMesh();
    const auto& vertices = mesh.getVertices();
    const auto& triangles = mesh.getTriangles();

    assert(!vertices.empty() && !triangles.empty());

    intersection.setMesh(vertices, triangles);

    SimulationConfig cfg;
    cfg.geometryFile = meshPath;
    cfg.rayCount = 300;
    cfg.reflectionRatio = 1.0;
    cfg.absorptionRatio = 0.0;
    cfg.energyLoss = 0.0;
    cfg.temperature = 1500.0;
    cfg.flowVelocity = Vector3(0, 1, 0);
    cfg.species["N2"] = SpeciesInfo{1.0e20, 4.65e-26};

    controller.setRaySourceArea(1.0);

    std::cout << "→ Erzeuge Rays in BoundingBox-Shell ...\n";

    controller.generateMixedRays(
        cfg,
        triangles,
        vertices,
        0.0,       // paddingFraction
        cfg.rayCount,
        cfg.rayCount
    );

    const auto& rays = controller.getRays();
    assert(!rays.empty());
    std::cout << "✔️  " << rays.size() << " Rays erzeugt.\n";

    for (const auto& ray : rays) {
        assert(ray.energy > 0.0);
        assert(ray.speciesMass > 0.0);
        assert(std::abs(ray.direction.norm() - 1.0) < 1e-6);
    }
}

int main() {
    test_simulation_controller_generates_shell_rays();
    return 0;
}

