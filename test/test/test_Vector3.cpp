#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Vector3.h"

TEST_CASE("Vector3 Konstruktor", "[Vector3]") {
    Vector3 v(1.0, 2.0, 3.0);
    REQUIRE(v.x == Approx(1.0));
    REQUIRE(v.y == Approx(2.0));
    REQUIRE(v.z == Approx(3.0));
}

TEST_CASE("Vector3 Addition und Subtraktion", "[Vector3]") {
    Vector3 a(1.0, 2.0, 3.0);
    Vector3 b(0.5, 1.0, -1.0);
    Vector3 sum = a + b;
    Vector3 diff = a - b;

    REQUIRE(sum.x == Approx(1.5));
    REQUIRE(sum.y == Approx(3.0));
    REQUIRE(sum.z == Approx(2.0));

    REQUIRE(diff.x == Approx(0.5));
    REQUIRE(diff.y == Approx(1.0));
    REQUIRE(diff.z == Approx(4.0));
}

TEST_CASE("Vector3 Skalierung und Norm", "[Vector3]") {
    Vector3 v(3.0, 4.0, 0.0);
    REQUIRE(v.norm() == Approx(5.0));

    Vector3 scaled = v * 2.0;
    REQUIRE(scaled.x == Approx(6.0));
    REQUIRE(scaled.y == Approx(8.0));
}

TEST_CASE("Vector3 Normalisierung", "[Vector3]") {
    Vector3 v(0.0, 3.0, 4.0);
    Vector3 n = v.normalize();
    REQUIRE(n.norm() == Approx(1.0));
    REQUIRE(n.x == Approx(0.0));
    REQUIRE(n.y == Approx(0.6).margin(1e-6));
    REQUIRE(n.z == Approx(0.8).margin(1e-6));
}

TEST_CASE("Vector3 Kreuzprodukt", "[Vector3]") {
    Vector3 a(1.0, 0.0, 0.0);
    Vector3 b(0.0, 1.0, 0.0);
    Vector3 c = a.cross(b);
    REQUIRE(c.x == Approx(0.0));
    REQUIRE(c.y == Approx(0.0));
    REQUIRE(c.z == Approx(1.0));
}

TEST_CASE("Vector3 Reflexion", "[Vector3]") {
    Vector3 d(1.0, -1.0, 0.0);
    Vector3 n(0.0, 1.0, 0.0);
    Vector3 r = d.reflect(n);
    REQUIRE(r.x == Approx(1.0));
    REQUIRE(r.y == Approx(1.0));
    REQUIRE(r.z == Approx(0.0));
}
