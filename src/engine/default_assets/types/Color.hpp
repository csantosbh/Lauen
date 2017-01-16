#pragma once

#include <cstdint>
#include "math/Vector.hpp"

namespace lau {

class Color {
public:
    Color();
    Color(uint32_t rgba);
    Color(float r, float g, float b, float a=1.0f);
    ~Color();

    void setRgba32U(uint32_t rgba);
    uint32_t getRgba32U() const;
    vec4& getRgbaV4f() {
        return color_;
    }
    float& r() {
        return color_[0];
    }
    float& g() {
        return color_[1];
    }
    float& b() {
        return color_[2];
    }
    float& a() {
        return color_[3];
    }

    static const Color red;
    static const Color green;
    static const Color blue;
    static const Color white;
    static const Color black;

private:
    vec4 color_;
};

} // namespace lau
