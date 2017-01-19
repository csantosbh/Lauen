#pragma once
#include <initializer_list>
#include "math/Vector.hpp"

namespace lau {

namespace math {

template<typename T>
T& map(void* ptr) {
    return *(T*)ptr;
}

enum class StorageOrder {RowMajor, ColumnMajor};

class mat3 {
public:

    mat3() {}
    mat3(const mat3& m);
    mat3(const std::initializer_list<float>& m,
         StorageOrder s = StorageOrder::RowMajor); // TODO make it the same as Eigen
    mat3(float a);
    mat3& operator=(const mat3& m);
    mat3& operator=(float a);
    static mat3 eye(float a);

    mat3 operator+(const mat3& m) const;
    mat3 operator+(float a) const;
    mat3& operator+=(const mat3& m);
    mat3& operator+=(float a);

    mat3 operator*(const mat3& m) const;
    mat3 operator*(float a) const;
    mat3& operator*=(const mat3& m);
    mat3& operator*=(float a);

    mat3& copyBlock(const mat3&, int row, int col);
    mat3& copyBlock(const vec3&, int row, int col);

    // Elementwise multiplication
    mat3 multiply(const mat3& m) const;
    mat3& multiplyInPlace(const mat3& m);

    mat3 transpose();
    mat3& transposeInPlace();

    mat3 inv() const;

    mat3 operator()(int row, int col) {
      return data[3*row+col];
    }

    union {
        vec3 rows[3];
        float data[3*3];
    };
};

class mat4 {
public:

    mat4() {}
    mat4(const mat4& m);
    mat4(const std::initializer_list<float>& m,
         StorageOrder s = StorageOrder::RowMajor); // TODO make it the same as Eigen
    mat4(float a);
    mat4& operator=(const mat4& m);
    mat4& operator=(float a);

    static mat4 eye(float a);
    static mat4 identity();

    mat4 operator+(const mat4& m) const;
    mat4 operator+(float a) const;
    mat4& operator+=(const mat4& m);
    mat4& operator+=(float a);

    mat4 operator*(const mat4& m) const;
    mat4 operator*(float a) const;
    mat4& operator*=(const mat4& m);
    mat4& operator*=(float a);

    vec4 operator*(const vec4&) const;

    mat4& copyBlock(const mat3&, int row, int col);
    mat4& copyBlock(const vec3&, int row, int col);
    mat4& copyBlock(const vec4&, int col);

    // Elementwise multiplication
    mat4 multiply(const mat4& m) const;
    mat4& multiplyInPlace(const mat4& m);

    mat4 transpose() const;
    mat4& transposeInPlace();

    mat4 inv() const;

    mat4 operator()(int row, int col) {
      return data[4*row+col];
    }

    union {
        vec4 rows[4];
        float data[4*4];
    };
};

} // namespace math

} // namespace lau
