#include <cassert>
#include <cmath>
#include <iostream>
#include "SurfaceInteractionModel.h"
#include "Ray.h"
#include "Vector3.h"
#include "HitInfo.h"
#include "ConfigLoader.h"

void test_reflection_is_computed_correctly() {
    SurfaceInteractionModel model(1.0, 0.2); // 100% Reflection, 20% Energy Loss

    Ray in;
    in.origin = {0, 0, 0};
    in.direction = {0, -1, 0};
    in.energy = 1.0;
    in.speciesMass = 1.0; // notwendig für Berechnung von Geschwindigkeit/Momentum

    HitInfo hit;
    hit.point = {0, -1, 0};
    hit.normal = {0, 1, 0};
    hit.panelId = 0;

    SimulationConfig cfg;
    cfg.model = ""; // Klassisches Modell ohne Sentman/DRIA
    cfg.energyLoss = 0.2;

    Ray out = model.generateReflection(cfg, in, hit);

    // Prüfe Eigenschaften, nicht exakte Werte
    assert(std::abs(out.direction.norm() - 1.0) < 1e-6); // Richtung normiert
    assert(out.direction.dot(hit.normal) > 0);             // Richtung zeigt weg von Oberfläche
    assert(std::abs(out.energy - 0.8) < 1e-6);              // 20% Energieverlust
    assert(out.active);                                    // Strahl aktiv

    std::cout << "[OK] test_reflection_is_computed_correctly\n";
}

int main() {
    test_reflection_is_computed_correctly();
    return 0;
}
