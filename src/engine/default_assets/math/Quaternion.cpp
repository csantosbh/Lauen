#include <algorithm>
#include <Eigen/Eigen>
#include <cmath>
#include "math/Quaternion.hpp"
#include "math/Math.hpp"

using namespace Eigen;
using namespace std;

namespace lau {

namespace math {

quaternion::quaternion(const quaternion& q) :
    w(q.w), x(q.x), y(q.y), z(q.z) {
}

quaternion::quaternion(float w, float x, float y, float z) :
    x(x), y(y), z(z), w(w) {
}

quaternion::quaternion(float angle, const vec3& axis) {
    w = cos(angle);
    vec3& v = map<vec3>(data);
    v = axis * sin(angle);
}

quaternion& quaternion::operator=(const quaternion& q) {
    x=q.x;
    y=q.y;
    z=q.z;
    w=q.w;
    return *this;
}

quaternion& quaternion::operator*=(const quaternion& q) {
    w = w*q.w - x*q.x - y*q.y - z*q.z;
    x = w*q.x + x*q.w + y*q.z - z*q.y;
    y = w*q.y + y*q.w + z*q.x - x*q.z;
    z = w*q.z + z*q.w + x*q.y - y*q.x;
}

quaternion quaternion::operator*(const quaternion& q) const {
    quaternion result(*this);

    result *= q;

    return result;
}

quaternion quaternion::identity() {
    return quaternion(0, 0, 0, 1);
}

quaternion quaternion::conjugate() const {
    return quaternion(w, -x, -y, -z);
}

quaternion& quaternion::conjugateInPlace() {
    x *= -1;
    y *= -1;
    z *= -1;

    return *this;
}

mat3 quaternion::matrix() const {
    mat3 result;
    Map<Matrix3f> r(result.data);
    r = Quaternionf(w, x, y, z).toRotationMatrix();

    return result;
}

vec3 quaternion::eulerAngles(int a, int b, int c) const {
    vec3 result;

    Map<Vector3f> r(result.data);
    r = Quaternionf(w, x, y, z).toRotationMatrix().eulerAngles(a, b, c);

    return result;
}

float quaternion::norm() const {
    return sqrt(x*x + y*y + z*z + w*w);
}

quaternion quaternion::lerp(float alpha, const quaternion& target) const {
    quaternion result;
    vec4& a = map<vec4>(result.data);
    const vec4& b = map<const vec4>(this->data);
    const vec4& c = map<const vec4>(target.data);

    a = b * alpha + c * (1-alpha);
    a.normalize();

    return result;
}

quaternion quaternion::slerp(float alpha, const quaternion& target) const {
    quaternion result;

    vec4& a = map<vec4>(result.data);
    const vec4& b = map<const vec4>(this->data);
    const vec4& c = map<const vec4>(target.data);

    float cos_theta = clamp(b.dot(c), -1.f, 1.f);
    const float EPSILON = 1e-6;

    if(abs(abs(cos_theta)-1.f) < EPSILON) {
        // Angle is too small, perform a lerp instead
        a = b * alpha + c * (1-alpha);
        a.normalize();
    } else {
        float theta = acos(cos_theta);

        a = (sinf( (1.0-alpha) * theta ) * b + sinf(alpha*theta) * c) / sinf(theta);
    }

    return result;
}

} // namespace math

} // namespace lau
