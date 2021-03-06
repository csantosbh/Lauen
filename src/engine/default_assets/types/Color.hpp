#pragma once

#include <cstdint>
#include <Eigen/Eigen>

namespace lau {

class Color {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Color();
    Color(uint32_t rgba);
    Color(float r, float g, float b, float a=1.0f);
    ~Color();

    void setRgba32U(uint32_t rgba);
    uint32_t getRgba32U() const;
    Eigen::Vector4f& getRgbaV4f() {
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
    Eigen::Vector4f color_;
};

} // namespace lau
