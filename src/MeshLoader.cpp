#include "MeshLoader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <cmath>

/**
 * @brief Load a mesh from a file.
 * 
 * This function delegates the loading based on file extension.
 * Currently supports only `.obj` files via TinyOBJLoader.
 * 
 * @param filename Path to the mesh file.
 * @return true if loading succeeds, false otherwise.
 */
bool MeshLoader::load(const std::string& filename) {
    if (filename.ends_with(".obj")) {
        return loadFromOBJ(filename);
    } else {
        std::cerr << "❌ Unknown format: " << filename << std::endl;
        return false;
    }
}

/**
 * @brief Compute the standard bounding box of the mesh.
 * 
 * Returns the axis-aligned bounding box (AABB) based on the vertex positions.
 * 
 * @return Pair of minimum and maximum 3D points.
 */
std::pair<Vector3, Vector3> MeshLoader::getBoundingBox() const {
    if (vertices.empty()) return {{0, 0, 0}, {0, 0, 0}};

    Vector3 min = vertices[0], max = vertices[0];
    for (const auto& v : vertices) {
        min.x = std::min(min.x, v.x); max.x = std::max(max.x, v.x);
        min.y = std::min(min.y, v.y); max.y = std::max(max.y, v.y);
        min.z = std::min(min.z, v.z); max.z = std::max(max.z, v.z);
    }
    return {min, max};
}

/**
 * @brief Load geometry from a Wavefront OBJ file.
 * 
 * Uses TinyOBJLoader to parse the file and build a list of vertices and triangles.
 * Also applies normal correction to ensure outward-facing triangles.
 * 
 * @param filename Path to .obj file
 * @return true on successful load, false otherwise.
 */
bool MeshLoader::loadFromOBJ(const std::string& filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
    if (!warn.empty()) std::cerr << "[TinyOBJ Warning] " << warn << std::endl;
    if (!err.empty()) std::cerr << "[TinyOBJ Error] " << err << std::endl;
    if (!ret) return false;

    // Load vertices
    vertices.clear();
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        vertices.emplace_back(Vector3{
            attrib.vertices[i + 0],
            attrib.vertices[i + 1],
            attrib.vertices[i + 2]
        });
    }

    // Load triangles (faces with 3 vertices)
    triangles.clear();
    int panelId = 0;
    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            size_t fv = shape.mesh.num_face_vertices[f];
            if (fv == 3) {
                int v0 = shape.mesh.indices[index_offset + 0].vertex_index;
                int v1 = shape.mesh.indices[index_offset + 1].vertex_index;
                int v2 = shape.mesh.indices[index_offset + 2].vertex_index;

                // Avoid degenerate triangles
                if (v0 != v1 && v1 != v2 && v2 != v0) {
                    triangles.emplace_back(Triangle{v0, v1, v2, panelId++});
                }
            }
            index_offset += fv;
        }
    }

    // === Correct triangle orientations so normals point outward ===
    if (!vertices.empty() && !triangles.empty()) {
        Vector3 center = getCenter(0.0);  // Center of bounding box

        for (auto& tri : triangles) {
            const Vector3& v0 = vertices[tri.v1];
            const Vector3& v1 = vertices[tri.v2];
            const Vector3& v2 = vertices[tri.v3];

            Vector3 faceCenter = (v0 + v1 + v2) / 3.0;
            Vector3 normal = (v1 - v0).cross(v2 - v0).normalize();
            Vector3 toCenter = (faceCenter - center).normalize();

            if (normal.dot(toCenter) < 0) {
                std::swap(tri.v1, tri.v2);  // Flip triangle winding
            }
        }
        std::cout << "✔️  Normals corrected: all faces point outward.\n";
    }

    return true;
}

/**
 * @brief Get reference to vertex list.
 */
const std::vector<Vector3>& MeshLoader::getVertices() const { return vertices; }

/**
 * @brief Get reference to triangle list.
 */
const std::vector<Triangle>& MeshLoader::getTriangles() const { return triangles; }

/**
 * @brief Compute padded bounding box.
 * 
 * Adds a padding fraction (e.g., 10%) of the box size in each direction.
 * Useful for enclosing regions slightly larger than the geometry.
 * 
 * @param paddingFraction Value in [0,1] indicating how much padding to add.
 * @return Min and max corners of padded bounding box.
 */
std::pair<Vector3, Vector3> MeshLoader::getBoundingBox(double paddingFraction) const {
    if (vertices.empty()) return {{0, 0, 0}, {0, 0, 0}};

    Vector3 min = vertices[0];
    Vector3 max = vertices[0];

    for (const auto& v : vertices) {
        min.x = std::min(min.x, v.x); max.x = std::max(max.x, v.x);
        min.y = std::min(min.y, v.y); max.y = std::max(max.y, v.y);
        min.z = std::min(min.z, v.z); max.z = std::max(max.z, v.z);
    }

    Vector3 size = max - min;
    Vector3 padding = size * paddingFraction;

    return {min - padding, max + padding};
}

/**
 * @brief Compute center of bounding box.
 * 
 * Optionally applies padding before computing the center.
 * 
 * @param paddingFraction Fraction used in bounding box computation.
 * @return Center point as Vector3.
 */
Vector3 MeshLoader::getCenter(double paddingFraction) const {
    auto [min, max] = getBoundingBox(paddingFraction);
    return (min + max) * 0.5;
}

/**
 * @brief Compute diagonal size (length) of bounding box.
 * 
 * This is the Euclidean distance between the min and max corners.
 * 
 * @param paddingFraction Optional padding before size computation.
 * @return Diagonal length as double.
 */
double MeshLoader::getSize(double paddingFraction) const {
    auto [min, max] = getBoundingBox(paddingFraction);
    Vector3 d = max - min;
    return std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
}

