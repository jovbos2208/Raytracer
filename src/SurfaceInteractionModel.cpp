#include "SurfaceInteractionModel.h"
#include "Ray.h"
#include "Vector3.h"
#include "ConfigLoader.h"
#include "IntersectionEngine.h"
#include <random>

/**
 * Utility: Generates a random number in the interval [0, 1)
 */
static double rand01() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(gen);
}

/**
 * Sets the mesh geometry that this interaction model will use.
 */
void SurfaceInteractionModel::setMesh(const std::vector<Vector3>& verts, const std::vector<Triangle>& tris) {
    vertices = verts;
    triangles = tris;
}

/**
 * Returns the area of a specific panel given by its ID.
 */
double SurfaceInteractionModel::getPanelArea(int panelId) const {
    if (panelId >= 0 && static_cast<size_t>(panelId) < triangleAreas.size()) {
        return triangleAreas[panelId];
    } else {
        return 0.0;
    }
}

/**
 * Generates a random cosine-weighted direction in the hemisphere around the given normal vector.
 */
Vector3 randomDiffuseDirection(const Vector3& normal) {
    double u1 = rand01();
    double u2 = rand01();

    double r = std::sqrt(u1);
    double theta = 2.0 * M_PI * u2;

    double x = r * std::cos(theta);
    double y = r * std::sin(theta);
    double z = std::sqrt(std::max(0.0, 1.0 - u1));

    // Local coordinate frame aligned to the normal
    Vector3 ez = normal.normalize();
    Vector3 ex = (std::abs(ez.x) > 0.9 ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).cross(ez).normalize();
    Vector3 ey = ez.cross(ex);

    return (ex * x + ey * y + ez * z).normalize();
}

/**
 * Generates a reflection ray using the DRIA model (Diffuse Reflection with Internal Accommodation).
 */
Ray SurfaceInteractionModel::generateDRIAReflection(
    const SimulationConfig& cfg,
    const Ray& incidentRay,
    const HitInfo& hit
) const {
    Vector3 n = hit.normal.normalize();
    Vector3 reflectedDir = randomDiffuseDirection(n);

    double T_w = 300.0; // wall temperature
    double E_i = incidentRay.energy;
    double T_i = (2.0 / 3.0) * E_i / cfg.kB;

    double alpha = cfg.energyAccommodation;
    double T_r = alpha * T_w + (1.0 - alpha) * T_i;

    double m = incidentRay.speciesMass;
    double newEnergy = 1.5 * cfg.kB * T_r;
    double v_mag = std::sqrt(2.0 * newEnergy / m);
    Vector3 newVelocity = reflectedDir * v_mag;

    Ray reflected;
    reflected.origin = hit.point + n * 1e-2; // small offset to avoid self-intersection
    reflected.direction = reflectedDir;
    reflected.velocity = newVelocity;
    reflected.energy = 0.5 * m * v_mag * v_mag;
    reflected.speciesMass = m;
    reflected.speciesDensity = incidentRay.speciesDensity;
    reflected.momentum = newVelocity * m;
    reflected.active = true;
    reflected.speciesName = incidentRay.speciesName;
    reflected.weight = incidentRay.weight;
    reflected.panelId = hit.panelId;

    return reflected;
}

/**
 * Generates a reflection ray using the Sentman model (mix of specular and diffuse).
 */
Ray SurfaceInteractionModel::generateSentmanReflection(
    const SimulationConfig& cfg,
    const Ray& incidentRay,
    const HitInfo& hit
) const {
    Vector3 n = hit.normal.normalize();

    double T_w = 300.0;
    double E_i = incidentRay.energy;
    double T_i = (2.0 / 3.0) * E_i / cfg.kB;

    double alpha = cfg.energyAccommodation;
    double T_r = alpha * T_w + (1.0 - alpha) * T_i;

    double m = incidentRay.speciesMass;
    double newEnergy = 1.5 * cfg.kB * T_r;
    double v_mag = std::sqrt(2.0 * newEnergy / m);

    double s = cfg.specularFraction;
    bool isSpecular = rand01() < s;

    Vector3 reflectedDir;
    if (isSpecular) {
        // Specular reflection
        Vector3 v_in = incidentRay.direction.normalize();
        reflectedDir = v_in - n * 2.0 * v_in.dot(n);
    } else {
        // Diffuse reflection
        reflectedDir = randomDiffuseDirection(n);
    }

    Vector3 newVelocity = reflectedDir * v_mag;

    Ray reflected;
    reflected.origin = hit.point + n * 1e-2;
    reflected.direction = reflectedDir;
    reflected.velocity = newVelocity;
    reflected.energy = 0.5 * m * v_mag * v_mag;
    reflected.speciesMass = m;
    reflected.speciesDensity = incidentRay.speciesDensity;
    reflected.momentum = newVelocity * m;
    reflected.active = true;
    reflected.speciesName = incidentRay.speciesName;
    reflected.weight = incidentRay.weight;
    reflected.panelId = hit.panelId;

    return reflected;
}

/**
 * Main function to generate a reflected ray based on the configured surface model.
 */
Ray SurfaceInteractionModel::generateReflection(
    const SimulationConfig& cfg,
    const Ray& incidentRay,
    const HitInfo& hit
) const {
    if (cfg.model == "DRIA") {
        return generateDRIAReflection(cfg, incidentRay, hit);
    }
    if (cfg.model == "Sentman") {
        return generateSentmanReflection(cfg, incidentRay, hit);
    }

    // Fallback basic reflection model
    double reflection_ratio = cfg.reflectionRatio;
    Vector3 n = hit.normal.normalize();
    Vector3 dir = incidentRay.direction;
    Vector3 reflectedDir;

    if (rand01() < reflection_ratio) {
        // Specular reflection
        reflectedDir = dir - n * 2.0 * dir.dot(n);
    } else {
        // Diffuse reflection
        reflectedDir = randomDiffuseDirection(n);
    }

    double newEnergy = incidentRay.energy * (1.0 - cfg.energyLoss);
    double v_mag = std::sqrt(2.0 * newEnergy / incidentRay.speciesMass);
    Vector3 newVelocity = reflectedDir.normalize() * v_mag;

    Ray reflected;
    reflected.origin = hit.point + n * 1e-2;
    reflected.direction = reflectedDir.normalize();
    reflected.velocity = newVelocity;
    reflected.energy = newEnergy;
    reflected.speciesMass = incidentRay.speciesMass;
    reflected.speciesDensity = incidentRay.speciesDensity;
    reflected.momentum = newVelocity * incidentRay.speciesMass;
    reflected.active = true;
    reflected.speciesName = incidentRay.speciesName;
    reflected.weight = incidentRay.weight;
    reflected.panelId = hit.panelId;

    return reflected;
}

/**
 * Constructor to initialize the surface model with specified reflection and absorption ratios.
 */
SurfaceInteractionModel::SurfaceInteractionModel(double reflection, double absorption)
    : reflectionRatio(reflection), absorptionRatio(absorption) {}

