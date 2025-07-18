#include "SimulationController.h"
#include "SurfaceInteractionModel.h"
#include "IntersectionEngine.h"
#include "Ray.h"
#include "MeshLoader.h"
#include "ConfigLoader.h"
#include "DragForceCalculator.h"
#include "MaxwellSampler.h"
#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include <omp.h>

// Generate a uniform random number between 0 and 1
static double rand01() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(gen);
}

// Sample a direction vector distributed with cosine-weighted hemispherical sampling around a normal
static Vector3 sampleCosineWeightedDirection(const Vector3& normal) {
    double u1 = rand01();
    double u2 = rand01();

    double r = std::sqrt(u1);
    double theta = 2.0 * M_PI * u2;

    double x = r * std::cos(theta);
    double y = r * std::sin(theta);
    double z = std::sqrt(1.0 - u1);

    Vector3 ez = normal.normalize();
    Vector3 ex = (std::abs(ez.x) > 0.9 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).cross(ez).normalize();
    Vector3 ey = ez.cross(ex);

    return (ex * x + ey * y + ez * z).normalize();
}

void SimulationController::setIntersectionEngine(IntersectionEngine* engine) {
    intersectionEngine = engine;
}

void SimulationController::loadMesh(const std::string& path) {
    if (!meshLoader.load(path)) {
        std::cerr << "❌ Error loading mesh: " << path << std::endl;
    } else {
        std::cout << "✔️  Mesh loaded successfully.\n";
    }
}

void SimulationController::setSurfaceModel(SurfaceInteractionModel* model) {
    surfaceModel = model;
}

Vector3 SimulationController::getTotalDragForce() const {
    return dragCalculator.getTotalDragForce();
}

/**
 * Generates rays distributed over a bounding box shell aligned with flow direction
 * @param config Configuration parameters
 * @param tris Triangle mesh
 * @param vertices Mesh vertex list
 * @param paddingFraction Padding around bounding box
 * @param rayCount Final number of rays to keep
 * @param totalRayCount Initial number of rays to generate
 */
void SimulationController::generateMixedRays(const SimulationConfig& config,
                                             const std::vector<Triangle>& tris,
                                             const std::vector<Vector3>& vertices,
                                             double paddingFraction,
                                             int rayCount,
                                             int totalRayCount) {
    if (!intersectionEngine) {
        std::cerr << "❌ No IntersectionEngine set.\n";
        return;
    }

    rays.clear();

    if (config.species.empty()) {
        std::cerr << "❌ No species specified in config.\n";
        return;
    }

    const Vector3 nFlux = config.flowVelocity.normalize();
    const double v_mag = config.flowVelocity.norm();

    // Construct local coordinate system aligned with flow direction
    Vector3 ex = nFlux;
    Vector3 tmp = std::abs(ex.z) < 0.99 ? Vector3{0, 0, 1} : Vector3{0, 1, 0};
    Vector3 ey = ex.cross(tmp).normalize();
    Vector3 ez = ex.cross(ey).normalize();

    // Compute bounding box of geometry
    Vector3 bbMin(
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max()
    );
    Vector3 bbMax(
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest()
    );

    for (const auto& v : vertices) {
        bbMin = Vector3::min(bbMin, v);
        bbMax = Vector3::max(bbMax, v);
    }

    Vector3 bbCenter = (bbMin + bbMax) * 0.5;
    Vector3 bbSize = bbMax - bbMin;
    double diag = bbSize.norm();

    // Compute dimensions for shell injection surface
    double flowPadding = 1 * diag;
    double sidePadding = 0.05 * diag;

    double halfU = 0.5 * std::abs(bbSize.dot(ey)) + sidePadding;
    double halfV = 0.5 * std::abs(bbSize.dot(ez)) + sidePadding;

    Vector3 centerFlux = bbCenter - ex * flowPadding;
    double A_flux = 4.0 * halfU * halfV;

    this->setRaySourceArea(A_flux);

    // Normalize species densities
    double sumDensity = 0.0;
    for (auto& [_, sp] : config.species) sumDensity += sp.density;

    std::vector<Ray> tmpRays;
    tmpRays.reserve(rayCount);

    std::mt19937 rng(1337);
    std::uniform_real_distribution<double> uni01(0.0, 1.0);

    // Ray generation per species
    for (auto& [name, sp] : config.species) {
        if (sp.mass <= 0.0 || sp.density <= 0.0) continue;

        int Nsp = std::round(totalRayCount * (sp.density / sumDensity));
        if (Nsp <= 0) continue;
        std::cout << "Nsp of " << name << ": " << Nsp << "\n";

        MaxwellSampler sampler(config.temperature, sp.mass, config.flowVelocity);

        for (int i = 0; i < Nsp; ++i) {
            double u = (uni01(rng) * 2.0 - 1.0) * halfU;
            double v = (uni01(rng) * 2.0 - 1.0) * halfV;
            Vector3 origin = centerFlux + u * ey + v * ez;

            Vector3 v_sample;
            do {
                v_sample = sampler.sampleVelocity();
            } while (v_sample.dot(nFlux) <= 0.0);  // Accept only forward moving rays

            double v_n = std::max(v_sample.dot(nFlux), 0.0);
            double weight = sp.density * v_n * A_flux / Nsp;

            Ray ray;
            ray.origin = origin;
            ray.velocity = v_sample;
            ray.direction = v_sample.normalize();
            ray.energy = 0.5 * sp.mass * v_sample.squaredNorm();
            ray.momentum = v_sample * sp.mass;
            ray.speciesMass = sp.mass;
            ray.speciesDensity = sp.density;
            ray.speciesName = name;
            ray.weight = weight;
            ray.panelId = -1;

            tmpRays.push_back(std::move(ray));
        }
    }

    // Normalize to requested ray count
    if (tmpRays.size() > static_cast<size_t>(rayCount))
        tmpRays.resize(rayCount);
    while (tmpRays.size() < static_cast<size_t>(rayCount))
        tmpRays.push_back(tmpRays[tmpRays.size() % tmpRays.size()]);

    rays = std::move(tmpRays);
    exportRayFieldVTK("ray_debug.vtk", tris, vertices);

    std::cout << "✅ Rays generated: " << rays.size() << "\n";
}

// === Additional utility functions ===

int SimulationController::getHitCount() const { return 0; }
int SimulationController::getBounceCount() const { return 0; }
int SimulationController::getRayCount() const { return rays.size(); }

const MeshLoader& SimulationController::getMesh() const { return meshLoader; }
std::vector<Ray>& SimulationController::getRays() { return rays; }

const std::map<int, PanelStats>& SimulationController::getPanelStats() const {
    return panelStats;
}

double SimulationController::getRaySourceArea() const {
    return raySourceArea;
}

void SimulationController::setRaySourceArea(double area) {
    raySourceArea = area;
}

// Export ray field and geometry in VTK format for visualization
void SimulationController::exportRayFieldVTK(const std::string& filename,
    const std::vector<Triangle>& tris,
    const std::vector<Vector3>& vertices) const {

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Failed to open VTK file: " << filename << "\n";
        return;
    }

    file << "# vtk DataFile Version 3.0\n";
    file << "Ray directions and geometry\n";
    file << "ASCII\n";
    file << "DATASET POLYDATA\n";

    size_t numGeometryPoints = vertices.size();
    size_t numRayPoints = rays.size();
    size_t totalPoints = numGeometryPoints + numRayPoints;

    file << "POINTS " << totalPoints << " float\n";
    for (const auto& v : vertices)
        file << v.x << " " << v.y << " " << v.z << "\n";
    for (const auto& ray : rays)
        file << ray.origin.x << " " << ray.origin.y << " " << ray.origin.z << "\n";

    file << "POLYGONS " << tris.size() << " " << tris.size() * 4 << "\n";
    for (const auto& tri : tris)
        file << "3 " << tri.v1 << " " << tri.v2 << " " << tri.v3 << "\n";

    file << "VERTICES " << numRayPoints << " " << numRayPoints * 2 << "\n";
    for (size_t i = 0; i < numRayPoints; ++i)
        file << "1 " << (numGeometryPoints + i) << "\n";

    file << "POINT_DATA " << totalPoints << "\n";
    file << "VECTORS ray_direction float\n";
    for (size_t i = 0; i < numGeometryPoints; ++i)
        file << "0 0 0\n";
    for (const auto& ray : rays)
        file << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << "\n";

    file.close();
    std::cout << "✅ Ray VTK file written: " << filename << "\n";
}

