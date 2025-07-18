#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vector3.h"
#include "Triangle.h"

class MeshLoader {
public:
    bool load(const std::string& filename);
    bool loadFromOBJ(const std::string& filename);

    const std::vector<Vector3>& getVertices() const;
    const std::vector<Triangle>& getTriangles() const;

    std::pair<Vector3, Vector3> getBoundingBox() const;
    std::pair<Vector3, Vector3> getBoundingBox(double paddingFraction) const;
    Vector3 getCenter(double paddingFraction) const;
    double getSize(double paddingFraction) const;

private:
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles;
};
