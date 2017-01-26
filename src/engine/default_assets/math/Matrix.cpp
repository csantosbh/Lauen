#include <algorithm>
#include <Eigen/Eigen>
#include "math/Matrix.hpp"
#include<iostream>

using namespace Eigen;
using namespace std;

namespace lau {

namespace math {

mat2::mat2(const mat2& m) {
    memcpy(data, m.data, 2*2*sizeof(float));
}

mat2::mat2(const std::array<float, 4>& m,
           StorageOrder inputRepresentation) {
    if(inputRepresentation == StorageOrder::ColumnMajor) {
        for(int j = 0; j < 2; ++j) {
            for(int i = 0; i < 2; ++i) {
                data[j*2+i] = m[i*2+j];
            }
        }
    } else {
        for(int i = 0; i < 4; ++i) {
            data[i] = m[i];
        }
    }
}

mat2::mat2(float a) {
    for(int i = 2*2-1; i >= 0; --i) {
        data[i] = a;
    }
}

mat2& mat2::operator=(const mat2& m) {
    memcpy(data, m.data, 2*2*sizeof(float));

    return *this;
}

mat2& mat2::operator=(float a) {
    for(int i = 2*2-1; i >= 0; --i) {
        data[i] = a;
    }

    return *this;
}

mat2 mat2::eye(float a) {
    mat2 m(0);
    for(int i = 0; i < 2; ++i) {
        m.data[i*2 + i] = a;
    }

    return m;
}

mat2 mat2::identity() {
    mat2 m(0);
    for(int i = 0; i < 2; ++i) {
        m.data[i*2 + i] = 1;
    }

    return m;
}

mat2 mat2::operator+(const mat2& m) const {
    mat2 result(*this);

    result += m;

    return result;
}

mat2 mat2::operator+(float a) const {
    mat2 result(*this);
    result += a;

    return result;
}

mat2& mat2::operator+=(const mat2& m) {
    Map<Matrix2f> a(this->data);
    Map<const Matrix2f> b(m.data);

    a += b;

    return *this;
}

mat2& mat2::operator+=(float a) {
    for(int i = 2*2-1; i >= 0; --i) {
        this->data[i] += a;
    }
    return *this;
}

mat2 mat2::operator*(const mat2& m) const {
    mat2 result(*this);

    result *= m;

    return result;
}

mat2 mat2::operator*(float a) const {
    mat2 result(*this);
    result *= a;

    return result;
}

mat2& mat2::operator*=(const mat2& m) {
    Map<Matrix2f> a(this->data);
    Map<const Matrix2f> b(m.data);

    a *= b;

    return *this;
}

mat2& mat2::operator*=(float a) {
    for(int i = 2*2-1; i >= 0; --i) {
        this->data[i] *= a;
    }
    return *this;
}

vec2 mat2::operator*(const vec2& v) const {
    vec2 result;

    Map<const Matrix2f> m(this->data);
    Map<Vector2f> a(result.data);
    Map<const Vector2f> b(v.data);

    a = m*b;

    return result;
}

mat2 mat2::multiply(const mat2& m) const {
    mat2 result(*this);

    result.multiplyInPlace(m);

    return result;
}

mat2& mat2::multiplyInPlace(const mat2& m) {
    Map<Array22f> a(this->data);
    Map<const Array22f> b(m.data);

    a *= b;

    return *this;
}

mat2 mat2::transpose() const {
    mat2 result(*this);
    result.transposeInPlace();

    return result;
}

mat2& mat2::transposeInPlace() {
    swap(data[0*3 + 1], data[1*3 + 0]);
    swap(data[1*3 + 0], data[0*3 + 1]);

    return *this;
}

mat2 mat2::inv() const {
    mat2 result(*this);

    Map<const Matrix2f> a(this->data);
    Map<Matrix2f> b(result.data);

    b = a.inverse();

    return result;
}

mat3::mat3(const mat3& m) {
    memcpy(data, m.data, 3*3*sizeof(float));
}

mat3::mat3(const array<float, 9>& m,
           StorageOrder inputRepresentation) {
    if(inputRepresentation == StorageOrder::ColumnMajor) {
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < 3; ++i) {
                data[j*3+i] = m[i*3+j];
            }
        }
    } else {
        for(int i = 0; i < 9; ++i) {
            data[i] = m[i];
        }
    }
}

mat3::mat3(float a) {
    for(int i = 3*3-1; i >= 0; --i) {
        data[i] = a;
    }
}

mat3& mat3::operator=(const mat3& m) {
    memcpy(data, m.data, 3*3*sizeof(float));

    return *this;
}

mat3& mat3::operator=(float a) {
    for(int i = 3*3-1; i >= 0; --i) {
        data[i] = a;
    }

    return *this;
}

mat3 mat3::eye(float a) {
    mat3 m(0);
    for(int i = 0; i < 3; ++i) {
        m.data[i*3 + i] = a;
    }

    return m;
}

mat3 mat3::identity() {
    mat3 m(0);
    for(int i = 0; i < 3; ++i) {
        m.data[i*3 + i] = 1;
    }

    return m;
}

mat3 mat3::operator+(const mat3& m) const {
    mat3 result(*this);

    result += m;

    return result;
}

mat3 mat3::operator+(float a) const {
    mat3 result(*this);
    result += a;

    return result;
}

mat3& mat3::operator+=(const mat3& m) {
    Map<Matrix3f> a(this->data);
    Map<const Matrix3f> b(m.data);

    a += b;

    return *this;
}

mat3& mat3::operator+=(float a) {
    for(int i = 3*3-1; i >= 0; --i) {
        this->data[i] += a;
    }
    return *this;
}

mat3 mat3::operator*(const mat3& m) const {
    mat3 result(*this);

    result *= m;

    return result;
}

mat3 mat3::operator*(float a) const {
    mat3 result(*this);
    result *= a;

    return result;
}

mat3& mat3::operator*=(const mat3& m) {
    Map<Matrix3f> a(this->data);
    Map<const Matrix3f> b(m.data);

    a *= b;

    return *this;
}

mat3& mat3::operator*=(float a) {
    for(int i = 3*3-1; i >= 0; --i) {
        this->data[i] *= a;
    }
    return *this;
}

vec3 mat3::operator*(const vec3& v) const {
    vec3 result;

    Map<const Matrix3f> m(this->data);
    Map<Vector3f> a(result.data);
    Map<const Vector3f> b(v.data);

    a = m*b;

    return result;
}

mat3& mat3::copyBlock(const mat2& m, int row, int col) {
    assert(row <= 1 && col <= 1);
    memcpy(&data[(row + 0)*3 + col], &m.data[0], 2*sizeof(float));
    memcpy(&data[(row + 1)*3 + col], &m.data[2], 2*sizeof(float));

    return *this;
}

mat3& mat3::copyBlock(const vec2& v, int row, int col) {
    assert(row <= 1 && col <= 2);

    data[(row + 0)*3 + col] = v.data[0];
    data[(row + 1)*3 + col] = v.data[1];

    return *this;
}

mat3& mat3::copyBlock(const vec3& v, int col) {
    assert(col <= 2);

    data[0*3 + col] = v.data[0];
    data[1*3 + col] = v.data[1];
    data[2*3 + col] = v.data[2];

    return *this;
}

mat3 mat3::multiply(const mat3& m) const {
    mat3 result(*this);

    result.multiplyInPlace(m);

    return result;
}

mat3& mat3::multiplyInPlace(const mat3& m) {
    Map<Array33f> a(this->data);
    Map<const Array33f> b(m.data);

    a *= b;

    return *this;
}

mat3 mat3::transpose() const {
    mat3 result(*this);
    result.transposeInPlace();

    return result;
}

mat3& mat3::transposeInPlace() {
    swap(data[0*3 + 1], data[1*3 + 0]); swap(data[0*3 + 2], data[2*3 + 0]);
    swap(data[1*3 + 0], data[0*3 + 1]); swap(data[1*3 + 2], data[2*3 + 1]);
    swap(data[2*3 + 0], data[0*3 + 2]); swap(data[2*3 + 1], data[1*3 + 2]);

    return *this;
}

mat3 mat3::inv() const {
    mat3 result(*this);

    Map<const Matrix3f> a(this->data);
    Map<Matrix3f> b(result.data);

    b = a.inverse();

    return result;
}

mat4::mat4(const mat4& m) {
    memcpy(data, m.data, 4*4*sizeof(float));
}

mat4::mat4(const array<float, 16> &m,
           StorageOrder inputRepresentation) {
    if(inputRepresentation == StorageOrder::ColumnMajor) {
        for(int j = 0; j < 4; ++j) {
            for(int i = 0; i < 4; ++i) {
                data[j*4+i] = m[i*4+j];
            }
        }
    } else {
        for(int i = 0; i < 16; ++i) {
            data[i] = m[i];
        }
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
    mat4 result(*this);

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
    mat4 result(*this);

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

    result.multiplyInPlace(m);

    return result;
}

mat4& mat4::multiplyInPlace(const mat4& m) {
    Map<Array44f> a(this->data);
    Map<const Array44f> b(m.data);

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
    mat4 result(*this);

    Map<const Matrix4f> a(this->data);
    Map<Matrix4f> b(result.data);

    b = a.inverse();

    return result;
}

ostream &operator<<(ostream &out, const mat3 &m) {
    for(int j = 0; j < 3; ++j) {
        for(int i = 0; i < 3; ++i) {
            out << m.data[j*3 + i] << " ";
        }
        out << endl;
    }
    return out;
}

ostream &operator<<(ostream &out, const mat4 &m) {
    for(int j = 0; j < 4; ++j) {
        for(int i = 0; i < 4; ++i) {
            out << m.data[j*4 + i] << " ";
        }
        out << endl;
    }
    return out;
}

} // namespace math

} // namespace lau
