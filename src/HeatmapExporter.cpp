#include "HeatmapExporter.h"
#include <fstream>
#include <iostream>

/// @brief Export a VTK file visualizing per-panel scalar values (e.g. temperature, force magnitude).
/// @param filename Output filename (e.g. "heatmap.vtk").
/// @param values Per-panel scalar values to be visualized (indexed by panel ID).
/// @param tris Triangular surface geometry (each with a panelId).
/// @param vertices Vertex coordinates.
void HeatmapExporter::exportHeatmapAsVTK(const std::string& filename,
                                         const std::map<int, double>& values,
                                         const std::vector<Triangle>& tris,
                                         const std::vector<Vector3>& vertices) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Could not open VTK output file: " << filename << "\n";
        return;
    }

    file << "# vtk DataFile Version 3.0\n";
    file << "Panel scalar heatmap\n";
    file << "ASCII\n";
    file << "DATASET POLYDATA\n";

    // === Write vertices ===
    file << "POINTS " << vertices.size() << " float\n";
    for (const auto& v : vertices) {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }

    // === Write triangle connectivity ===
    file << "POLYGONS " << tris.size() << " " << tris.size() * 4 << "\n";
    for (const auto& tri : tris) {
        file << "3 " << tri.v1 << " " << tri.v2 << " " << tri.v3 << "\n";
    }

    // === Write scalar values for each triangle (cell data) ===
    file << "CELL_DATA " << tris.size() << "\n";
    file << "SCALARS panel_scalar float 1\n";
    file << "LOOKUP_TABLE default\n";

    for (const auto& tri : tris) {
        auto it = values.find(tri.panelId);
        double val = (it != values.end()) ? it->second : 0.0;
        file << val << "\n";
    }

    file.close();
    std::cout << "✅ Heatmap VTK file written: " << filename << "\n";
}

/// @brief Export rays as lines to a VTK file (e.g., for debugging or visualization).
/// @param filename Output file name.
/// @param raySegments Each ray segment is a pair of (start point, end point).
/// @param vertices Geometry vertices.
/// @param tris Triangular geometry.
/// @param lineScale Scale factor for line thickness (not used in ASCII VTK).
void HeatmapExporter::exportRaysAsVTK(const std::string& filename,
                                      const std::vector<std::pair<Vector3, Vector3>>& raySegments,
                                      const std::vector<Vector3>& vertices,
                                      const std::vector<Triangle>& tris,
                                      double lineScale) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Could not open VTK ray output file: " << filename << "\n";
        return;
    }

    file << "# vtk DataFile Version 3.0\n";
    file << "Ray trace visualization\n";
    file << "ASCII\n";
    file << "DATASET POLYDATA\n";

    size_t numPoints = vertices.size() + raySegments.size() * 2;

    // === Write points (geometry + ray start and end points) ===
    file << "POINTS " << numPoints << " float\n";
    for (const auto& v : vertices) {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }
    for (const auto& seg : raySegments) {
        file << seg.first.x << " " << seg.first.y << " " << seg.first.z << "\n";
        file << seg.second.x << " " << seg.second.y << " " << seg.second.z << "\n";
    }

    // === Write geometry triangles ===
    file << "POLYGONS " << tris.size() << " " << tris.size() * 4 << "\n";
    for (const auto& tri : tris) {
        file << "3 " << tri.v1 << " " << tri.v2 << " " << tri.v3 << "\n";
    }

    // === Write ray lines (each as a polyline with 2 points) ===
    size_t numLines = raySegments.size();
    file << "LINES " << numLines << " " << numLines * 3 << "\n";
    for (size_t i = 0; i < numLines; ++i) {
        size_t idx0 = vertices.size() + i * 2;
        size_t idx1 = idx0 + 1;
        file << "2 " << idx0 << " " << idx1 << "\n";
    }

    file.close();
    std::cout << "✅ Ray trace VTK file written: " << filename << "\n";
}

