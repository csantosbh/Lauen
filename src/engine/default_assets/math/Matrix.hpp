#pragma once
#include <initializer_list>
#include "math/Vector.hpp"

namespace lau {

namespace math {

enum class StorageOrder {RowMajor, ColumnMajor};

class mat2 {
public:

    mat2() {}
    mat2(const mat2& m);
    mat2(const std::initializer_list<float>& m,
         StorageOrder s = StorageOrder::RowMajor); // TODO make it the same as Eigen
    mat2(float a);
    mat2& operator=(const mat2& m);
    mat2& operator=(float a);

    static mat2 eye(float a);
    static mat2 identity();

    mat2 operator+(const mat2& m) const;
    mat2 operator+(float a) const;
    mat2& operator+=(const mat2& m);
    mat2& operator+=(float a);

    mat2 operator*(const mat2& m) const;
    mat2 operator*(float a) const;
    mat2& operator*=(const mat2& m);
    mat2& operator*=(float a);

    vec2 operator*(const vec2&) const;

    // Elementwise multiplication
    mat2 multiply(const mat2& m) const;
    mat2& multiplyInPlace(const mat2& m);

    mat2 transpose() const;
    mat2& transposeInPlace();

    mat2 inv() const;

    mat2 operator()(int row, int col) {
      return data[2*row+col];
    }

    union {
        vec2 rows[2];
        float data[2*2];
    };
};

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
    static mat3 identity();

    mat3 operator+(const mat3& m) const;
    mat3 operator+(float a) const;
    mat3& operator+=(const mat3& m);
    mat3& operator+=(float a);

    mat3 operator*(const mat3& m) const;
    mat3 operator*(float a) const;
    mat3& operator*=(const mat3& m);
    mat3& operator*=(float a);

    vec3 operator*(const vec3&) const;

    mat3& copyBlock(const mat2&, int row, int col);
    mat3& copyBlock(const vec2&, int row, int col);
    mat3& copyBlock(const vec3&, int col);

    // Elementwise multiplication
    mat3 multiply(const mat3& m) const;
    mat3& multiplyInPlace(const mat3& m);

    mat3 transpose() const;
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
