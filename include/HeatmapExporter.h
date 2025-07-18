#pragma once
#include "Ray.h"
#include <string>
#include <vector>
#include "Vector3.h"
#include "Triangle.h"

class HeatmapExporter {
public:
    static void exportVTK(const std::string& filename,
                          const std::vector<Vector3>& vertices,
                          const std::vector<Triangle>& triangles,
                          const std::vector<double>& scalars);  // z. B. Kraftbeträge pro Panel
                          
    static void exportRaysAsVTK(const std::string& filename,
                            const std::vector<std::pair<Vector3, Vector3>>& raySegments,
                            const std::vector<Vector3>& vertices,
                            const std::vector<Triangle>& triangles,
                            double length);

                            
    // Füge dies in HeatmapExporter.h hinzu:
	static void exportSceneWithCdAndRays(
	    const std::string& filename,
	    const std::vector<Vector3>& vertices,
	    const std::vector<Triangle>& triangles,
	    const std::vector<double>& scalars,
	    const std::vector<std::pair<Vector3, Vector3>>& raySegments,
	    double rayLength);

           
};

