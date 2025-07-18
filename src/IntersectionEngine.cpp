#include "IntersectionEngine.h"
#include "MeshLoader.h"
#include <iostream>
#include <limits>
#include <cmath>

/**
 * @brief Assigns the triangle mesh used for ray intersections.
 * 
 * This function stores the vertex and triangle data for the mesh
 * against which incoming rays will be tested.
 * 
 * @param verts A list of 3D vertex positions.
 * @param tris A list of triangles, defined by indices into the vertex list.
 */
void IntersectionEngine::setMesh(const std::vector<Vector3>& verts, const std::vector<Triangle>& tris) {
    vertices = verts;
    triangles = tris;
}

/**
 * @brief Computes the nearest intersection between a ray and the loaded mesh.
 * 
 * Iterates through all triangles and returns the closest intersection point, if any.
 * 
 * @param ray The ray to trace.
 * @return Optional HitInfo object containing the hit point, normal, and metadata.
 */
std::optional<HitInfo> IntersectionEngine::intersect(const Ray& ray) const {
    std::optional<HitInfo> closestHit;
    double closestT = std::numeric_limits<double>::infinity();  // Start with max distance
    const double epsilon = 1e-12;  // Ignore very close hits (e.g., self-intersection)

    for (const auto& tri : triangles) {
        const Vector3& v0 = vertices[tri.v1];
        const Vector3& v1 = vertices[tri.v2];
        const Vector3& v2 = vertices[tri.v3];

        double t;
        if (!intersects(ray, v0, v1, v2, t)) continue;  // No intersection with this triangle
        if (t < epsilon || t >= closestT) continue;    // Not closer than current closest

        // Compute exact intersection point and normal
        Vector3 intersection = ray.origin + ray.direction * t;
        Vector3 normal = (v1 - v0).cross(v2 - v0).normalize();

        // Flip normal if pointing in the same direction as the ray (backface culling)
        if (normal.dot(ray.direction) > 0) normal = normal * -1.0;

        closestT = t;
        closestHit = HitInfo{
            .point    = intersection,
            .normal   = normal,
            .panelId  = tri.panelId,
            .nextRay  = {},         // To be filled later
            .t        = t
        };
    }

    return closestHit;
}

/**
 * @brief Ray-triangle intersection test using the Möller–Trumbore algorithm.
 * 
 * Tests whether a given ray intersects with a triangle defined by three vertices.
 * 
 * @param ray The ray to test.
 * @param v0 First vertex of the triangle.
 * @param v1 Second vertex of the triangle.
 * @param v2 Third vertex of the triangle.
 * @param t Output distance to intersection point along the ray.
 * @return true if the ray intersects the triangle, false otherwise.
 */
bool IntersectionEngine::intersects(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2, double& t) const {
    const double EPSILON = 1e-8;

    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;
    Vector3 h = ray.direction.cross(edge2);
    double a = edge1.dot(h);

    if (std::abs(a) < EPSILON) return false;  // Ray is parallel to triangle

    double f = 1.0 / a;
    Vector3 s = ray.origin - v0;
    double u = f * s.dot(h);
    if (u < 0.0 || u > 1.0) return false;

    Vector3 q = s.cross(edge1);
    double v = f * ray.direction.dot(q);
    if (v < 0.0 || u + v > 1.0) return false;

    t = f * edge2.dot(q);  // Distance along ray
    return t > EPSILON;
}

