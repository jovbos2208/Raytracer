#pragma once
#include "MeshLoader.h"
#include <vector>
#include <optional>
#include "Vector3.h"
#include "Ray.h"
#include "Triangle.h"
#include "HitInfo.h"

class IntersectionEngine {
    public:
        void setMesh(const std::vector<Vector3>& verts, const std::vector<Triangle>& tris);
    
        std::optional<HitInfo> intersect(const Ray& ray) const;
    
        
    
    private:
        std::vector<Vector3> vertices;
        std::vector<Triangle> triangles;
        
        bool intersects(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2, double& t) const;

    };
