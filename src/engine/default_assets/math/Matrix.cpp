#include <algorithm>
#include <Eigen/Eigen>
#include "math/Matrix.hpp"

using namespace Eigen;
using namespace std;

namespace lau {

namespace math {

mat4::mat4(const mat4& m) {
    memcpy(data, m.data, 4*4*sizeof(float));
}

mat4::mat4(const std::initializer_list<float>& m,
           StorageOrder s) {
    int i = 0;
    for(const auto& element: m) {
        data[i++] = element;
    }
}

mat4::mat4(float a) {
    for(int i = 4*4-1; i >= 0; --i) {
        data[i] = a;
    }
}

mat4& mat4::operator=(const mat4& m) {
    memcpy(data, m.data, 4*4*sizeof(float));

    return *this;
}

mat4& mat4::operator=(float a) {
    for(int i = 4*4-1; i >= 0; --i) {
        data[i] = a;
    }

    return *this;
}

mat4 mat4::eye(float a) {
    mat4 m(0);
    for(int i = 0; i < 4; ++i) {
        m.data[i*4 + i] = a;
    }

    return m;
}

mat4 mat4::identity() {
    mat4 m(0);
    for(int i = 0; i < 4; ++i) {
        m.data[i*4 + i] = 1;
    }

    return m;
}

mat4 mat4::operator+(const mat4& m) const {
    mat4 result;

    result += m;

    return result;
}

mat4 mat4::operator+(float a) const {
    mat4 result(*this);
    result += a;

    return result;
}

mat4& mat4::operator+=(const mat4& m) {
    Map<Matrix4f> a(this->data);
    Map<const Matrix4f> b(m.data);

    a += b;

    return *this;
}

mat4& mat4::operator+=(float a) {
    for(int i = 4*4-1; i >= 0; --i) {
        this->data[i] += a;
    }
    return *this;
}

mat4 mat4::operator*(const mat4& m) const {
    mat4 result;

    result *= m;

    return result;
}

mat4 mat4::operator*(float a) const {
    mat4 result(*this);
    result *= a;

    return result;
}

mat4& mat4::operator*=(const mat4& m) {
    Map<Matrix4f> a(this->data);
    Map<const Matrix4f> b(m.data);

    a *= b;

    return *this;
}

mat4& mat4::operator*=(float a) {
    for(int i = 4*4-1; i >= 0; --i) {
        this->data[i] *= a;
    }
    return *this;
}

vec4 mat4::operator*(const vec4& v) const {
    vec4 result;

    Map<const Matrix4f> m(this->data);
    Map<Vector4f> a(result.data);
    Map<const Vector4f> b(v.data);

    a = m*b;

    return result;
}

mat4& mat4::copyBlock(const mat3& m, int row, int col) {
    assert(row <= 1 && col <= 1);
    memcpy(&data[(row + 0)*4 + col], &m.data[0], 3*sizeof(float));
    memcpy(&data[(row + 1)*4 + col], &m.data[3], 3*sizeof(float));
    memcpy(&data[(row + 2)*4 + col], &m.data[6], 3*sizeof(float));

    return *this;
}

mat4& mat4::copyBlock(const vec3& v, int row, int col) {
    assert(row <= 1 && col <= 3);

    data[(row + 0)*4 + col] = v.data[0];
    data[(row + 1)*4 + col] = v.data[1];
    data[(row + 2)*4 + col] = v.data[2];

    return *this;
}

mat4& mat4::copyBlock(const vec4& v, int col) {
    assert(col <= 3);

    data[0*4 + col] = v.data[0];
    data[1*4 + col] = v.data[1];
    data[2*4 + col] = v.data[2];
    data[2*4 + col] = v.data[3];

    return *this;
}

mat4 mat4::multiply(const mat4& m) const {
    mat4 result(*this);

    result *= m;

    return result;
}

mat4& mat4::multiplyInPlace(const mat4& m) {
    Map<Array4f> a(this->data);
    Map<const Array4f> b(m.data);

    a *= b;

    return *this;
}

mat4 mat4::transpose() const {
    mat4 result(*this);
    result.transposeInPlace();

    return result;
}

mat4& mat4::transposeInPlace() {
    swap(data[0*4 + 1], data[1*4 + 0]); swap(data[0*4 + 2], data[2*4 + 0]); swap(data[0*4 + 3], data[3*4 + 0]);
    swap(data[1*4 + 0], data[0*4 + 1]); swap(data[1*4 + 2], data[2*4 + 1]); swap(data[1*4 + 3], data[3*4 + 1]);
    swap(data[2*4 + 0], data[0*4 + 2]); swap(data[2*4 + 1], data[1*4 + 2]); swap(data[2*4 + 3], data[3*4 + 2]);
    swap(data[3*4 + 0], data[0*4 + 3]); swap(data[3*4 + 1], data[1*4 + 3]); swap(data[3*4 + 2], data[2*4 + 3]);

    return *this;
}

mat4 mat4::inv() const {
    mat4 result;

    Map<const Matrix4f> a(this->data);
    Map<Matrix4f> b(result.data);

    b = a.inverse();

    return result;
}

} // namespace math

} // namespace lau
