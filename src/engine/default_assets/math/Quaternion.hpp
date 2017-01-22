#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"

namespace lau {

namespace math {

class quaternion {
public:
    quaternion() {}
    quaternion(const quaternion& q);
    quaternion(float w, float x, float y, float z);
    // Angle in radians
    quaternion(float angle, const vec3& axis);
    quaternion& operator=(const quaternion& q);

    quaternion& operator*=(const quaternion& q);
    quaternion operator*(const quaternion& q) const;

    static quaternion identity();
    quaternion conjugate() const;
    quaternion& conjugateInPlace();
    mat3 matrix() const;
    vec3 eulerAngles(int a, int b, int c) const;

    float norm() const;

    quaternion lerp(float alpha, const quaternion& target) const;
    quaternion slerp(float alpha, const quaternion& target) const;

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    union {
        float data[4];
        struct {
            float x;
            float y;
            float z;
            float w; // Real part
        };
    };
};

} // namespace math

} // namespace lau
