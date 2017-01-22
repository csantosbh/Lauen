#include <Eigen/Eigen>
#include "math/Vector.hpp"

using namespace Eigen;

namespace lau {

namespace math {

////
// Vector2
vec2::vec2(const vec2& v) : x(v.x),
                            y(v.y) {
}

vec2::vec2(float x, float y) : x(x),
                               y(y) {
}

vec2::vec2(const std::initializer_list<float>& array) {
    assert(array.size() == 2);
    int j = 0;
    for(const auto& i: array) {
        data[j++] = i;
    }
}

vec2::vec2(float a) : x(a),
                      y(a) {
}

vec2& vec2::operator=(const vec2& v) {
    x = v.x;
    y = v.y;
    return *this;
}

vec2& vec2::operator=(float a) {
    x = y = a;
    return *this;
}

vec2 vec2::operator+(const vec2& v) const {
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

vec2 vec2::operator+(float a) const {
    vec2 result;
    result += a;
    return result;
}

vec2& vec2::operator+=(float a) {
    x += a;
    y += a;
    return *this;
}

vec2 vec2::operator-(const vec2& v) const {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    Map<const Vector2f> b(v.data);
    r-=b;

    return res;
}

vec2& vec2::operator-=(const vec2& v) {
    Map<Vector2f> r(this->data);
    Map<const Vector2f> b(v.data);
    r-=b;
    return *this;
}

vec2 vec2::operator-(float a) const {
    vec2 result;
    result -= a;
    return result;
}

vec2& vec2::operator-=(float a) {
    x -= a;
    y -= a;
    return *this;
}

vec2 vec2::operator-() const {
    return vec2(-x, -y);
}

vec2& vec2::operator*=(float a) {
    Map<Vector2f> r(this->data);
    r*=a;
    return *this;
}

vec2 vec2::operator*(float a) const {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    r*=a;
    return res;
}

vec2 operator*(float a, const vec2& v) {
    return v*a;
}

// Elementwise multiplication
vec2 vec2::multiply(const vec2& v) const {
    vec2 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    return res;
}

float vec2::dot(const vec2& v) const {
    Map<const Vector2f> a(this->data);
    Map<const Vector2f> b(v.data);
    return a.dot(b);
}

vec2 vec2::normalized() const {
    vec2 result(*this);
    result.normalize();
    return result;
}

vec2& vec2::normalize() {
    Map<Vector2f> r(this->data);
    r.normalize();
    return *this;
}

vec2& vec2::operator/=(float a) {
    Map<Vector2f> r(this->data);
    r /= a;
    return *this;
}

vec2 vec2::operator/(float a) const {
    vec2 res(*this);
    Map<Vector2f> r(res.data);
    r /= a;
    return res;
}

const vec3& vec2::homogeneous() {
    _h = 1.0;

    return *reinterpret_cast<const vec3*>(this);
}

vec3 vec2::homogeneous() const {
    return vec3(x,y,1);
}


////
// Vector3
vec3::vec3(const vec3& v) : x(v.x),
                            y(v.y),
                            z(v.z) {
}

vec3::vec3(float x, float y, float z) : x(x),
                                        y(y),
                                        z(z) {
}

vec3::vec3(const std::initializer_list<float>& array) {
    assert(array.size() == 3);
    int j = 0;
    for(const auto& i: array) {
        data[j++] = i;
    }
}

vec3::vec3(float a) : x(a),
                      y(a),
                      z(a) {
}

vec3& vec3::operator=(const vec3& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

vec3& vec3::operator=(float a) {
    x = y = z = a;
    return *this;
}

vec3 vec3::operator+(const vec3& v) const {
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

vec3 vec3::operator+(float a) const {
    vec3 result;
    result += a;
    return result;
}

vec3& vec3::operator+=(float a) {
    x += a;
    y += a;
    z += a;
    return *this;
}

vec3 vec3::operator-(const vec3& v) const {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    Map<const Vector3f> b(v.data);
    r-=b;

    return res;
}

vec3& vec3::operator-=(const vec3& v) {
    Map<Vector3f> r(this->data);
    Map<const Vector3f> b(v.data);
    r-=b;
    return *this;
}

vec3 vec3::operator-(float a) const {
    vec3 result;
    result -= a;
    return result;
}

vec3& vec3::operator-=(float a) {
    x -= a;
    y -= a;
    z -= a;
    return *this;
}

vec3 vec3::operator-() const {
    return vec3(-x, -y, -z);
}

vec3& vec3::operator*=(float a) {
    Map<Vector3f> r(this->data);
    r*=a;
    return *this;
}

vec3 vec3::operator*(float a) const {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    r*=a;
    return res;
}

vec3 operator*(float a, const vec3& v) {
    return v*a;
}

// Elementwise multiplication
vec3 vec3::multiply(const vec3& v) const {
    vec3 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    res.z *= v.z;
    return res;
}

float vec3::dot(const vec3& v) const {
    Map<const Vector3f> a(this->data);
    Map<const Vector3f> b(v.data);
    return a.dot(b);
}

vec3 vec3::cross(const vec3& v) const {
    vec3 result(y*v.z - z*v.y,
                z*v.x - x*v.z,
                x*v.y - y*v.z);

    return result;
}

vec3 vec3::normalized() const {
    vec3 result(*this);
    result.normalize();
    return result;
}

vec3& vec3::normalize() {
    Map<Vector3f> r(this->data);
    r.normalize();
    return *this;
}

vec3& vec3::operator/=(float a) {
    Map<Vector3f> r(this->data);
    r /= a;
    return *this;
}

const vec4& vec3::homogeneous() {
    _h = 1.0;

    return *reinterpret_cast<const vec4*>(this);
}

vec4 vec3::homogeneous() const {
    return vec4(x,y,z,1);
}

vec3 vec3::operator/(float a) const {
    vec3 res(*this);
    Map<Vector3f> r(res.data);
    r /= a;
    return res;
}

////
// Vector4
vec4::vec4(const vec4& v) : x(v.x),
                            y(v.y),
                            z(v.z),
                            w(v.w) {
}

vec4::vec4(float x, float y, float z, float w) : x(x),
                                                 y(y),
                                                 z(z),
                                                 w(w) {
}

vec4::vec4(const std::initializer_list<float>& array) {
    assert(array.size() == 4);
    int j = 0;
    for(const auto& i: array) {
        data[j++] = i;
    }
}

vec4::vec4(float a) : x(a),
                      y(a),
                      z(a),
                      w(a) {
}

vec4& vec4::operator=(const vec4& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
    return *this;
}

vec4& vec4::operator=(float a) {
    x = y = z = w = a;
    return *this;
}

vec4 vec4::operator+(const vec4& v) const {
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

vec4 vec4::operator+(float a) const {
    vec4 result;
    result += a;
    return result;
}

vec4& vec4::operator+=(float a) {
    x += a;
    y += a;
    z += a;
    w += a;
    return *this;
}

vec4 vec4::operator-(const vec4& v) const {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    Map<const Vector4f> b(v.data);
    r-=b;

    return res;
}

vec4& vec4::operator-=(const vec4& v) {
    Map<Vector4f> r(this->data);
    Map<const Vector4f> b(v.data);
    r-=b;
    return *this;
}

vec4 vec4::operator-(float a) const {
    vec4 result;
    result -= a;
    return result;
}

vec4& vec4::operator-=(float a) {
    x -= a;
    y -= a;
    z -= a;
    w -= a;
    return *this;
}

vec4 vec4::operator-() const {
    return vec4(-x, -y, -z, -w);
}

vec4& vec4::operator*=(float a) {
    Map<Vector4f> r(this->data);
    r*=a;
    return *this;
}

vec4 vec4::operator*(float a) const {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    r*=a;
    return res;
}

vec4 operator*(float a, const vec4& v) {
    return v*a;
}

vec4 vec4::multiply(const vec4& v) const {
    vec4 res(*this);
    res.x *= v.x;
    res.y *= v.y;
    res.z *= v.z;
    res.w *= v.w;
    return res;
}

float vec4::dot(const vec4& v) const {
    Map<const Vector4f> a(this->data);
    Map<const Vector4f> b(v.data);
    return a.dot(b);
}

vec4 vec4::normalized() const {
    vec4 result(*this);
    result.normalize();
    return result;
}

vec4& vec4::normalize() {
    Map<Vector4f> r(this->data);
    r.normalize();
    return *this;
}

vec4& vec4::operator/=(float a) {
    Map<Vector4f> r(this->data);
    r /= a;
    return *this;
}

vec4 vec4::operator/(float a) const {
    vec4 res(*this);
    Map<Vector4f> r(res.data);
    r /= a;
    return res;
}

}

} // namespace lau
