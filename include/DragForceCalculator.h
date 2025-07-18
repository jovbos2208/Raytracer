#ifndef DRAG_FORCE_CALCULATOR_H
#define DRAG_FORCE_CALCULATOR_H

#include "Vector3.h"
#include "Ray.h"
#include "Triangle.h"
#include <map>
#include <vector>
#include <string>

struct PanelForce {
    Vector3 force = {0, 0, 0};
    double area   = 0.0;
};

class DragForceCalculator {
public:
    void setMesh(const std::vector<Vector3>& verts,
                 const std::vector<Triangle>& tris);

    void accumulateForce(const Ray& in, const Ray& out, double area);

    void merge(const DragForceCalculator& other);

    Vector3 getTotalDragForce() const { return totalForce; }
    
    Vector3 computeScaledForce(double totalMassFlux) const;

    void exportPanelForcesCSV(const std::string& filename) const;

    const std::map<int, PanelForce>& getPanelForces() const {
        return perPanelForces;
    }

private:
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles;

    std::map<int, PanelForce> perPanelForces;
    Vector3 totalForce = {0, 0, 0};

    double computeTriangleArea(const Triangle& tri) const;
};

#endif // DRAG_FORCE_CALCULATOR_H

