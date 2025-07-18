#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "HitInfo.h"
#include "ConfigLoader.h"
#include "Triangle.h"
#include <vector>

class SurfaceInteractionModel {
public:
    SurfaceInteractionModel(double reflection = 1.0, double absorption = 0.0);

    Ray generateReflection(const SimulationConfig& cfg, const Ray& incidentRay, const HitInfo& hit) const;

    void setMesh(const std::vector<Vector3>& verts, const std::vector<Triangle>& tris);

    double getPanelArea(int panelId) const;

private:
    Ray generateDRIAReflection(const SimulationConfig& cfg, const Ray& incidentRay, const HitInfo& hit) const;
    Ray generateSentmanReflection(const SimulationConfig& cfg, const Ray& incidentRay, const HitInfo& hit) const;

    double reflectionRatio;
    double absorptionRatio;

    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles;
    std::vector<double> triangleAreas; // Fläche je Panel
};
