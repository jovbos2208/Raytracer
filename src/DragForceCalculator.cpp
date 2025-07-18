#include "DragForceCalculator.h"
#include <iostream>

/// @brief Accumulate drag force contributions from a single ray interaction.
/// @param incidentRay Incoming ray before surface hit.
/// @param reflectedRay Reflected ray after surface interaction.
/// @param panelArea Area of the surface panel the ray hit.
void DragForceCalculator::accumulateForce(const Ray& incidentRay, const Ray& reflectedRay, double panelArea) {
    // Compute momentum difference (Δp)
    Vector3 deltaP = reflectedRay.momentum - incidentRay.momentum;

    // Scale by ray's statistical weight
    Vector3 weightedForce = deltaP * incidentRay.weight;

    // Total accumulated force
    totalForce += weightedForce;

    // Accumulate per-panel force
    perPanelForces[incidentRay.panelId] += weightedForce;

    // Store this ray pair for debugging or analysis (optional)
    rayContributions.push_back({incidentRay, reflectedRay, panelArea});
}

/// @brief Return the total accumulated drag force vector.
/// @return Vector3 total force.
Vector3 DragForceCalculator::getTotalDragForce() const {
    return totalForce;
}

/// @brief Get all per-panel forces (map from panel ID to total force on that panel).
/// @return Map of panel ID to force vector.
std::map<int, Vector3> DragForceCalculator::getPanelForces() const {
    return perPanelForces;
}

/// @brief Merge data from another DragForceCalculator instance.
/// @param other The other instance to merge into this one.
void DragForceCalculator::merge(const DragForceCalculator& other) {
    totalForce += other.totalForce;

    for (const auto& [id, force] : other.perPanelForces) {
        perPanelForces[id] += force;
    }

    rayContributions.insert(rayContributions.end(),
                            other.rayContributions.begin(),
                            other.rayContributions.end());
}

/// @brief Computes a scaled drag force based on total incoming mass flux.
/// @param totalMassFlux The physical mass flux from all rays (kg/s).
/// @return Scaled force vector in [N].
Vector3 DragForceCalculator::computeScaledForce(double totalMassFlux) const {
    // Compute total unscaled force magnitude (i.e., sum of ray contributions)
    double summedWeights = 0.0;
    for (const auto& [_, force] : perPanelForces) {
        summedWeights += force.norm();  // Could alternatively use ray weight sum
    }

    // If weights available, scale force to match physical mass flux
    if (summedWeights > 0.0)
        return totalForce * (totalMassFlux / summedWeights);
    else
        return totalForce;  // No scaling possible
}

