#pragma once
#include "math/Vector.hpp"

namespace lau {

class mat4 {
public:
    mat4() {}
    mat4(const mat4& m);
    mat4(const std::initializer_list<float>& m);
    mat4(float a);
    mat4& operator=(const mat4& m);
    mat4& operator=(float a);
    static mat4 eye(float a);

    mat4 operator+(const mat4& m) const;
    mat4 operator+(float a) const;
    mat4& operator+=(const mat4& m);
    mat4& operator+=(float a);

    mat4 operator*(const mat4& m) const;
    mat4 operator*(float a) const;
    mat4& operator*=(const mat4& m);
    mat4& operator*=(float a);

    // Elementwise multiplication
    mat4 multiply(const mat4& m);
    mat4& multiplyInPlace(const mat4& m);

    mat4 inv();

    mat4 operator()(int row, int col) {
      return data[4*row+col];
    }

    union {
        vec4 rows[4];
        float data[4*4];
    };
};

} // namespace lau
