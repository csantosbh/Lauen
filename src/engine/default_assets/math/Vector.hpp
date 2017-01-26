#pragma once

#include <array>

namespace lau {

namespace math {

class vec3;
class vec4;

class vec2 {
public:
    vec2() {}
    vec2(const vec2& v);
    vec2(float x, float y);
    vec2(const std::array<float, 2> &array);
    vec2(float a);
    vec2& operator=(const vec2& v);
    vec2& operator=(float a);

    vec2 operator+(const vec2& v) const;
    vec2& operator+=(const vec2& v);
    vec2 operator+(float a) const;
    vec2& operator+=(float a);

    vec2 operator-(const vec2& v) const;
    vec2& operator-=(const vec2& v);
    vec2 operator-(float a) const;
    vec2& operator-=(float a);
    vec2 operator-() const;

    vec2& operator*=(float a);
    vec2 operator*(float a) const;
    // Elementwise multiplication
    vec2 multiply(const vec2& v) const;
    float dot(const vec2& v) const;

    float norm() const;

    vec2& operator/=(float a);
    vec2 operator/(float a) const;

    // TODO rename to homogeneousInPlace
    const vec3& homogeneous();
    vec3 homogeneous() const;

    vec2 normalized() const;
    vec2& normalize();

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    float data[3]; // Third element is the homogeneous coordinate

    float& x() {
        return data[0];
    }
    float& y() {
        return data[1];
    }

    const float& x() const {
        return data[0];
    }
    const float& y() const {
        return data[1];
    }
};

class vec3 {
public:
    vec3() {}
    vec3(const vec3& v);
    vec3(float x, float y, float z);
    vec3(const std::array<float, 3>& array);
    vec3(float a);
    vec3& operator=(const vec3& v);
    vec3& operator=(float a);

    vec3 operator+(const vec3& v) const;
    vec3& operator+=(const vec3& v);
    vec3 operator+(float a) const;
    vec3& operator+=(float a);

    vec3 operator-(const vec3& v) const;
    vec3& operator-=(const vec3& v);
    vec3 operator-(float a) const;
    vec3& operator-=(float a);
    vec3 operator-() const;

    vec3& operator*=(float a);
    vec3 operator*(float a) const;
    // Elementwise multiplication
    vec3 multiply(const vec3& v) const;
    float dot(const vec3& v) const;

    vec3 cross(const vec3&) const;

    vec3& operator/=(float a);
    vec3 operator/(float a) const;

    const vec4& homogeneous();
    vec4 homogeneous() const;

    vec3 normalized() const;
    vec3& normalize();

    float norm() const;

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    float data[4]; // Fourth coordinate is the homogeneous coordinate
    float& x() {
      return data[0];
    }
    float& y() {
      return data[1];
    }
    float& z() {
      return data[2];
    }

    const float& x() const {
        return data[0];
    }
    const float& y() const {
        return data[1];
    }
    const float& z() const {
        return data[2];
    }
};

class vec4 {
public:
    vec4() { }
    vec4(const vec4& v);
    vec4(float x, float y, float z, float w);
    vec4(const std::array<float, 4>& array);
    vec4(float a);
    vec4& operator=(const vec4& v);
    vec4& operator=(float a);

    vec4 operator+(const vec4& v) const;
    vec4& operator+=(const vec4& v);
    vec4 operator+(float a) const;
    vec4& operator+=(float a);

    vec4 operator-(const vec4& v) const;
    vec4& operator-=(const vec4& v);
    vec4 operator-(float a) const;
    vec4& operator-=(float a);
    vec4 operator-() const;

    vec4& operator*=(float a);
    vec4 operator*(float a) const;
    // Elementwise multiplication
    vec4 multiply(const vec4& v) const;
    float dot(const vec4& v) const;

    vec4 normalized() const;
    vec4& normalize();

    float norm() const;

    vec4& operator/=(float a);
    vec4 operator/(float a) const;

    const vec3& head();
    vec3 head() const;

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    float data[4];
    float& x() {
        return data[0];
    }
    float& y() {
      return data[1];
    }
    float& z() {
      return data[2];
    }
    float& w() {
      return data[3];
    }

    const float& x() const {
        return data[0];
    }
    const float& y() const {
        return data[1];
    }
    const float& z() const {
        return data[2];
    }
    const float& w() const {
        return data[3];
    }
};

vec2 operator*(float a, const vec2& v);
vec3 operator*(float a, const vec3& v);
vec4 operator*(float a, const vec4& v);

} // namespace math

} // namespace lau
