#include <Eigen/Eigen>
#include "math/Vector.hpp"

using namespace Eigen;

namespace lau {

////
// Vector2
vec2::vec2() { }

vec2::vec2(const vec2& v) {
    x=v.x;
    y=v.y;
}

vec2::vec2(float a) {
    x = y = a;
}

vec2& vec2::operator=(const vec2& v) {
    x=v.x;
    y=v.y;
    return *this;
}

vec2& vec2::operator=(float a) {
    x = y = a;
    return *this;
}

vec2 vec2::operator+(const vec2& v) {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    Map<const Vector2f> b(v.data);
    r+=b;

    return res;
}

vec2& vec2::operator+=(const vec2& v) {
    Map<Vector2f> r(this->data);
    Map<const Vector2f> b(v.data);
    r+=b;
    return *this;
}

vec2& vec2::operator+=(float a) {
    x+=a;
    y+=a;
    return *this;
}

vec2& vec2::operator*=(float a) {
    Map<Vector2f> r(this->data);
    r*=a;
    return *this;
}

vec2 vec2::operator*(float a) {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    r*=a;
    return res;
}
// Elementwise multiplication
vec2 vec2::multiply(const vec2& v) {
    vec2 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    return res;
}

float vec2::dot(const vec2& v) {
    Map<Vector2f> a(this->data);
    Map<const Vector2f> b(v.data);
    return a.dot(b);
}

vec2& vec2::operator/=(float a) {
    Map<Vector2f> r(this->data);
    r /= a;
    return *this;
}

vec2 vec2::operator/(float a) {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    r /= a;
    return res;
}

////
// Vector3
vec3::vec3() { }

vec3::vec3(const vec3& v) {
    x=v.x;
    y=v.y;
    z=v.z;
}

vec3::vec3(float a) {
    x = y = z = a;
}

vec3& vec3::operator=(const vec3& v) {
    x=v.x;
    y=v.y;
    z=v.z;
    return *this;
}

vec3& vec3::operator=(float a) {
    x = y = z = a;
    return *this;
}

vec3 vec3::operator+(const vec3& v) {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    Map<const Vector3f> b(v.data);
    r+=b;

    return res;
}

vec3& vec3::operator+=(const vec3& v) {
    Map<Vector3f> r(this->data);
    Map<const Vector3f> b(v.data);
    r+=b;
    return *this;
}

vec3& vec3::operator+=(float a) {
    x+=a;
    y+=a;
    z+=a;
    return *this;
}

vec3& vec3::operator*=(float a) {
    Map<Vector3f> r(this->data);
    r*=a;
    return *this;
}

vec3 vec3::operator*(float a) {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    r*=a;
    return res;
}
// Elementwise multiplication
vec3 vec3::multiply(const vec3& v) {
    vec3 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    res.z *= v.z;
    return res;
}

float vec3::dot(const vec3& v) {
    Map<Vector3f> a(this->data);
    Map<const Vector3f> b(v.data);
    return a.dot(b);
}

vec3& vec3::operator/=(float a) {
    Map<Vector3f> r(this->data);
    r /= a;
    return *this;
}

vec3 vec3::operator/(float a) {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    r /= a;
    return res;
}

////
// Vector4
vec4::vec4() { }

vec4::vec4(const vec4& v) {
    x=v.x;
    y=v.y;
    z=v.z;
    w=v.w;
}

vec4::vec4(float a) {
    x = y = z = w = a;
}

vec4& vec4::operator=(const vec4& v) {
    x=v.x;
    y=v.y;
    z=v.z;
    w=v.w;
    return *this;
}

vec4& vec4::operator=(float a) {
    x = y = z = w = a;
    return *this;
}

vec4 vec4::operator+(const vec4& v) {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    Map<const Vector4f> b(v.data);
    r+=b;

    return res;
}

vec4& vec4::operator+=(const vec4& v) {
    Map<Vector4f> r(this->data);
    Map<const Vector4f> b(v.data);
    r+=b;
    return *this;
}

vec4& vec4::operator+=(float a) {
    x+=a;
    y+=a;
    z+=a;
    w+=a;
    return *this;
}

vec4& vec4::operator*=(float a) {
    Map<Vector4f> r(this->data);
    r*=a;
    return *this;
}

vec4 vec4::operator*(float a) {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    r*=a;
    return res;
}

vec4 vec4::multiply(const vec4& v) {
    vec4 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    res.z *= v.z;
    res.w *= v.w;
    return res;
}

float vec4::dot(const vec4& v) {
    Map<Vector4f> a(this->data);
    Map<const Vector4f> b(v.data);
    return a.dot(b);
}

vec4& vec4::operator/=(float a) {
    Map<Vector4f> r(this->data);
    r /= a;
    return *this;
}

vec4 vec4::operator/(float a) {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    r /= a;
    return res;
}

} // namespace lau
