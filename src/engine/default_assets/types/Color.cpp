#include "Color.hpp"

using namespace std;

namespace lau {

const Color Color::red(0xff0000ff);
const Color Color::green(0xff0000ff);
const Color Color::blue(0xff0000ff);
const Color Color::white(0xff0000ff);
const Color Color::black(0xff0000ff);

Color::Color() {
    setRgba32U(0xffffffff);
}

Color::Color(uint32_t rgba) {
    setRgba32U(rgba);
}

void Color::setRgba32U(uint32_t rgba) {
    color_[0] = static_cast<float>((rgba&0xff000000)>>24)/255.0f;
    color_[1] = static_cast<float>((rgba&0x00ff0000)>>16)/255.0f;
    color_[2] = static_cast<float>((rgba&0x0000ff00)>> 8)/255.0f;
    color_[3] = static_cast<float>((rgba&0x000000ff)    )/255.0f;
}

uint32_t Color::getRgba32U() const {
    const float& r = color_[0];
    const float& g = color_[1];
    const float& b = color_[2];
    const float& a = color_[3];
    return (static_cast<uint32_t>(r*255.0f))<<24 |
           (static_cast<uint32_t>(g*255.0f))<<16 |
           (static_cast<uint32_t>(b*255.0f))<< 8 |
            static_cast<uint32_t>(a*255.0f);
}

Color::Color(float r, float g, float b, float a) {
    color_[0] = r;
    color_[1] = g;
    color_[2] = b;
    color_[3] = a;
}

Color::~Color() {
}

} // namespace lau

