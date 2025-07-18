#pragma once

#include "Vector3.h"
#include "Ray.h"

struct HitInfo {
    Vector3 point;       // Schnittpunkt
    Vector3 normal;      // Normale am Schnittpunkt
    int panelId;         // ID des Panels, das getroffen wurde
    Ray nextRay;         // Optional: vorberechneter reflektierter Ray (optional verwendet)
    double t;            // Abstand entlang des Strahls
};
