#include <gtest/gtest.h>
#include "IntersectionEngine.h"
#include "MeshLoader.h"
#include "Ray.h"
#include "Vector3.h"
#include "Triangle.h"

class IntersectionEngineTest : public ::testing::Test {
protected:
    IntersectionEngine engine;

    void SetUp() override {
        MeshLoader loader;
        ASSERT_TRUE(loader.load("models/Cube.obj")) << "❌ Cube.obj konnte nicht geladen werden";

        const auto& vertices = loader.getVertices();
        const auto& triangles = loader.getTriangles();
        ASSERT_FALSE(vertices.empty());
        ASSERT_FALSE(triangles.empty());

        engine.setMesh(vertices, triangles);
    }
};

TEST_F(IntersectionEngineTest, RayHitsCubeFromAbove) {
    Ray ray;
    ray.origin = Vector3(0.5, 0.5, 2.0);  // über der Mitte des Würfels
    ray.direction = Vector3(0, 0, -1).normalize();  // nach unten
    ray.panelId = -1;

    auto hit = engine.intersect(ray);
    ASSERT_TRUE(hit.has_value()) << "❌ Ray sollte Oberfläche treffen";

    EXPECT_NEAR(hit->point.x, 0.5, 1e-4);
    EXPECT_NEAR(hit->point.y, 0.5, 1e-4);
    EXPECT_NEAR(hit->point.z, 1.0, 1e-4);  // Oberseite bei z=1?
}

TEST_F(IntersectionEngineTest, RayMissesCubeFromSide) {
    Ray ray;
    ray.origin = Vector3(-1.0, 0.5, 0.5);  // Links vom Würfel
    ray.direction = Vector3(-1, 0, 0).normalize();  // Weiter nach links
    ray.panelId = -1;

    auto hit = engine.intersect(ray);
    EXPECT_FALSE(hit.has_value()) << "❌ Ray sollte daneben fliegen";
}

TEST_F(IntersectionEngineTest, RayHitsCorner) {
    Ray ray;
    ray.origin = Vector3(0.0, 0.0, 2.0);   // über Ecke
    ray.direction = Vector3(0, 0, -1).normalize();
    ray.panelId = -1;

    auto hit = engine.intersect(ray);
    ASSERT_TRUE(hit.has_value()) << "❌ Ray sollte Ecke treffen";
    EXPECT_NEAR(hit->point.x, 0.0, 1e-4);
    EXPECT_NEAR(hit->point.y, 0.0, 1e-4);
    EXPECT_NEAR(hit->point.z, 1.0, 1e-4);
}

