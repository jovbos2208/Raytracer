#pragma once

#include "Vector3.h"

struct PanelStats {
    Vector3 totalImpulse = {0, 0, 0};  // aufsummierte Impulsänderung
    int hitCount = 0;                 // Anzahl der Ray-Treffer

    // optional: Mittlere Impulsänderung pro Treffer
    Vector3 averageImpulse() const {
        return hitCount > 0 ? totalImpulse * (1.0 / hitCount) : Vector3{0, 0, 0};
    }

    double impulseMagnitude() const {
        return totalImpulse.norm();
    }
};

