#pragma once
#include "Vector3.h"
#include <vector>

struct Triangle {
    int v1, v2, v3;
    Vector3 normal;
    int panelId = 0; // optional: z.B. zur Gruppierung

    // Berechnet die Fläche des Dreiecks mit gegebenen Vertex-Koordinaten
    double area(const std::vector<Vector3>& vertices) const {
        const Vector3& a = vertices[v1];
        const Vector3& b = vertices[v2];
        const Vector3& c = vertices[v3];
        return 0.5 * ((b - a).cross(c - a)).norm();
    }

    Triangle() : v1(0), v2(0), v3(0), panelId(-1) {}

    Triangle(int a, int b, int c, int pid = -1)
        : v1(a), v2(b), v3(c), panelId(pid) {}
};

