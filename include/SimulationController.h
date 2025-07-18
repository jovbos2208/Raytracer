#pragma once
#include "DragForceCalculator.h"
#include "Vector3.h"
#include "Triangle.h"
#include "Ray.h"
#include "PanelStats.h"
#include "MeshLoader.h"
#include <vector>
#include <map>
#include <string>

class SurfaceInteractionModel;
class IntersectionEngine;
struct SimulationConfig;

class SimulationController {
public:
    void loadMesh(const std::string& path);
    void setSurfaceModel(SurfaceInteractionModel* model);
    void setIntersectionEngine(IntersectionEngine* engine);

    void generateMixedRays(const SimulationConfig& config,
                           const std::vector<Triangle>& triangles,
                           const std::vector<Vector3>& vertices,
                           double paddingFraction,
                           int rayCount,
                           int totalRayCount); // <- NEU


    int getHitCount() const;
    int getBounceCount() const;
    int getRayCount() const;

    const MeshLoader& getMesh() const;
    std::vector<Ray>& getRays();
    const std::map<int, PanelStats>& getPanelStats() const;

    Vector3 getTotalDragForce() const;
    double getRaySourceArea() const;
    void setRaySourceArea(double area);

    void exportRayFieldVTK(const std::string& filename,const std::vector<Triangle>& tris,
        const std::vector<Vector3>& vertices) const;


private:
    MeshLoader meshLoader;
    SurfaceInteractionModel* surfaceModel = nullptr;
    IntersectionEngine* intersectionEngine = nullptr;

    std::vector<Ray> rays;
    std::map<int, PanelStats> panelStats;

    double raySourceArea = 1.0;

    DragForceCalculator dragCalculator;
};
