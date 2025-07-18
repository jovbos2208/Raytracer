#include "MeshLoader.h"
#include <cassert>
#include <iostream>
#include <cmath>

void test_meshloader_asymmetric_bounding_box() {
    const std::string filename = "models/Cube.obj";

    MeshLoader loader;
    bool loaded = loader.load(filename);
    assert(loaded && "❌ Mesh konnte nicht geladen werden");

    const auto& verts = loader.getVertices();
    const auto& tris = loader.getTriangles();
    assert(!verts.empty());
    assert(!tris.empty());

    // Normale Bounding Box ohne Padding
    auto [min, max] = loader.getBoundingBox(0.0);
    Vector3 diag = max - min;
    double baseSize = diag.norm();
    std::cout << "✔️  Standard-Bounding Box Größe: " << baseSize << "\n";

    // Bounding Box mit Padding
    double paddingFraction = 0.2;
    auto [amin, amax] = loader.getBoundingBox(paddingFraction);
    Vector3 paddedDiag = amax - amin;
    double paddedSize = paddedDiag.norm();

    std::cout << "🔍 Asymmetrische Bounding Box (mit Padding):\n";
    std::cout << "    Min: (" << amin.x << ", " << amin.y << ", " << amin.z << ")\n";
    std::cout << "    Max: (" << amax.x << ", " << amax.y << ", " << amax.z << ")\n";

    // Vergrößerung sollte deutlich erkennbar sein
    assert(paddedSize > baseSize * (1.0 + paddingFraction * 0.5));

    std::cout << "[OK] Asymmetrische Bounding Box korrekt erweitert ✅\n";
}

int main() {
    test_meshloader_asymmetric_bounding_box();
    return 0;
}

