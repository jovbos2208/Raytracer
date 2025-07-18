#pragma once
#include <cmath>
#include <iostream>
#include <algorithm>

struct Vector3 {
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(double s) const { return {x / s, y / s, z / s}; }

    double dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }

    Vector3 cross(const Vector3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    Vector3 normalize() const {
        double len = norm();
        if (len == 0.0) return {0.0, 0.0, 0.0};
        return *this * (1.0 / len);
    }

    double norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    double squaredNorm() const {
        return x * x + y * y + z * z;
    }

    Vector3 reflect(const Vector3& normal) const {
        return *this - normal * (2.0 * this->dot(normal));
    }

    Vector3 operator-() const { return {-x, -y, -z}; }

    double& operator[](size_t i) {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    const double& operator[](size_t i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    // Elementweises Minimum
    static Vector3 min(const Vector3& a, const Vector3& b) {
        return {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z)
        };
    }

    // Elementweises Maximum
    static Vector3 max(const Vector3& a, const Vector3& b) {
        return {
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z)
        };
    }
};

// --- double * Vector3 (extern, damit auch 2.0 * v funktioniert)
inline Vector3 operator*(double s, const Vector3& v) {
    return Vector3(v.x * s, v.y * s, v.z * s);
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << v.x << " " << v.y << " " << v.z;
    return os;
}

inline bool operator==(const Vector3& a, const Vector3& b) {
    const double eps = 1e-8;
    return std::abs(a.x - b.x) < eps &&
           std::abs(a.y - b.y) < eps &&
           std::abs(a.z - b.z) < eps;
}

inline bool operator!=(const Vector3& a, const Vector3& b) {
    return !(a == b);
}
