#pragma once

namespace lau {

class vec2 {
public:
    vec2() {}
    vec2(const vec2& v);
    vec2(float x, float y);
    vec2(float a);
    vec2& operator=(const vec2& v);
    vec2& operator=(float a);

    vec2 operator+(const vec2& v);
    vec2& operator+=(const vec2& v);
    vec2& operator+=(float a);

    vec2& operator*=(float a);
    vec2 operator*(float a);
    // Elementwise multiplication
    vec2 multiply(const vec2& v);
    float dot(const vec2& v);

    vec2& operator/=(float a);
    vec2 operator/(float a);

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    union {
        float data[2];
        struct {
            float x;
            float y;
        };
    };
};

class vec3 {
public:
    vec3() {}
    vec3(const vec3& v);
    vec3(float x, float y, float z);
    vec3(float a);
    vec3& operator=(const vec3& v);
    vec3& operator=(float a);

    vec3 operator+(const vec3& v);
    vec3& operator+=(const vec3& v);
    vec3& operator+=(float a);

    vec3& operator*=(float a);
    vec3 operator*(float a);
    // Elementwise multiplication
    vec3 multiply(const vec3& v);
    float dot(const vec3& v);

    vec3& operator/=(float a);
    vec3 operator/(float a);

    float& operator[](int i) {
      return data[i];
    }

    const float& operator[](int i) const {
      return data[i];
    }

    union {
        float data[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
};

class vec4 {
public:
    vec4() { }
    vec4(const vec4& v);
    vec4(float x, float y, float z, float w);
    vec4(float a);
    vec4& operator=(const vec4& v);
    vec4& operator=(float a);

    vec4 operator+(const vec4& v);
    vec4& operator+=(const vec4& v);
    vec4& operator+=(float a);

    vec4& operator*=(float a);
    vec4 operator*(float a);
    // Elementwise multiplication
    vec4 multiply(const vec4& v);
    float dot(const vec4& v);

    vec4& operator/=(float a);
    vec4 operator/(float a);

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
            float w;
        };
    };
};

} // namespace lau
