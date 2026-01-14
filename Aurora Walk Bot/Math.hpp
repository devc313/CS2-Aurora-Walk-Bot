#ifndef MATH_HPP
#define MATH_HPP

#include <algorithm>
#include <limits>
#include <cmath>
#include <initializer_list>


struct Vector3g {
    float x, y, z;

    Vector3g() : x(0), y(0), z(0) {}
    Vector3g(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3g operator-(const Vector3g& other) const {
        return Vector3g(x - other.x, y - other.y, z - other.z);
    }

    float dot(const Vector3g& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3g cross(const Vector3g& other) const {
        return Vector3g(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

struct Triangle {
    int a, b, c;
    Triangle() : a(0), b(0), c(0) {}
    Triangle(int a_, int b_, int c_) : a(a_), b(b_), c(c_) {}
};

struct AABB {
    Vector3g min;
    Vector3g max;

    bool RayIntersects(const Vector3g& rayOrigin, const Vector3g& rayDir) const {
        float tmin = std::numeric_limits<float>::lowest();
        float tmax = std::numeric_limits<float>::max();

        const float* rayOriginArr = &rayOrigin.x;
        const float* rayDirArr = &rayDir.x;
        const float* minArr = &min.x;
        const float* maxArr = &max.x;

        for (int i = 0; i < 3; ++i) {
            float invDir = 1.0f / rayDirArr[i];
            float t0 = (minArr[i] - rayOriginArr[i]) * invDir;
            float t1 = (maxArr[i] - rayOriginArr[i]) * invDir;

            if (invDir < 0.0f) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
        }

        return tmax >= tmin && tmax >= 0;
    }
};

struct TriangleCombined {
    Vector3g v0, v1, v2;

    TriangleCombined() = default;
    TriangleCombined(const Vector3g& v0_, const Vector3g& v1_, const Vector3g& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {
    }

    AABB ComputeAABB() const {
        Vector3g min_point, max_point;

        min_point.x = std::min({ v0.x, v1.x, v2.x });
        min_point.y = std::min({ v0.y, v1.y, v2.y });
        min_point.z = std::min({ v0.z, v1.z, v2.z });

        max_point.x = std::max({ v0.x, v1.x, v2.x });
        max_point.y = std::max({ v0.y, v1.y, v2.y });
        max_point.z = std::max({ v0.z, v1.z, v2.z });

        return { min_point, max_point };
    }
};

#endif