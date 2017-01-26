#if defined(NACL)
#include "window/NaCl.hpp"

namespace pp { // TODO tem que ser no namespace pp?
    Module* CreateModule() {
        return new lau::NaClCanvasModule();
    }
}
#else

#if defined(DESKTOP)
#include "window/Desktop.hpp"
#elif defined(JAVASCRIPT)
#include "window/Browser.hpp"
#endif

#include "math/Math.hpp"
#include "math/Matrix.hpp"
#include "math/Vector.hpp"
#include "math/Quaternion.hpp"

using namespace std;
using namespace lau::math;

float randf() {
    const int MAX_RAND = 1e6;
    const float MAX_INTENSITY = 10.0;
    return static_cast<float>(rand()%MAX_RAND - MAX_RAND/2)/static_cast<float>(MAX_RAND) * MAX_INTENSITY;
}

mat2 randmat2() {
    mat2 result;
    for(int i = 2*2 - 1; i >= 0; --i) {
        result.data[i] = randf();
    }
    return result;
}

int cmp2(const mat2& a, const mat2& b) {
    float result = 0;
    for(int i = 2*2 - 1; i >= 0; --i) {
        result += fabs(a.data[i] - b.data[i]);
    }
    return result;
}

mat3 randmat3() {
    mat3 result;
    for(int i = 3*3 - 1; i >= 0; --i) {
        result.data[i] = randf();
    }
    return result;
}

int cmp3(const mat3& a, const mat3& b) {
    float result = 0;
    for(int i = 3*3 - 1; i >= 0; --i) {
        result += fabs(a.data[i] - b.data[i]);
    }
    return result;
}

mat4 randmat4() {
    mat4 result;
    for(int i = 4*4 - 1; i >= 0; --i) {
        result.data[i] = randf();
    }
    return result;
}

int cmp4(const mat4& a, const mat4& b) {
    float result = 0;
    for(int i = 4*4 - 1; i >= 0; --i) {
        result += fabs(a.data[i] - b.data[i]);
    }
    return result;
}

vec2 randvec2() {
    return {randf(), randf()};
}

vec3 randvec3() {
    return {randf(), randf(), randf()};
}

vec4 randvec4() {
    return {randf(), randf(), randf(), randf()};
}

void print(const mat2& m) {
    for(int j = 0; j < 2; ++j) {
        for(int i = 0; i < 2; ++i) {
            cout << m(j, i) << " ";
        }
        cout << endl;
    }
}

void test_math() {
    {
    mat2 a = randmat2();
    assert(cmp2(a * a.inv(), mat2::identity()) <= 1e-5);
    vec2 v = randvec2();
    auto v2 = a.inv() * v;
    assert((a*v2-v).norm() <= 1e-5);
    assert(abs((a*v2).norm()-v.norm()) <= 1e-5);
    }
    {
    mat3 a = randmat3();
    assert(cmp3(a * a.inv(), mat3::identity()) <= 1e-5);
    vec3 v = randvec3();
    auto v2 = a.inv() * v;
    assert((a*v2-v).norm() <= 1e-5);
    assert(abs((a*v2).norm()-v.norm()) <= 1e-5);
    }
    {
    mat4 a = randmat4();
    assert(cmp4(a * a.inv(), mat4::identity()) <= 1e-5);
    vec4 v = randvec4();
    auto v2 = a.inv() * v;
    assert((a*v2-v).norm() <= 1e-5);
    assert(abs((a*v2).norm()-v.norm()) <= 1e-5);
    }

    int xulambs = 0;
    xulambs++;
}

int main(void)
{
    try {
#if defined(DESKTOP)
        lau::Desktop window;
#elif defined(JAVASCRIPT)
        lau::Browser window;
#endif
        test_math();
        // Initialize window
        window.init(640, 480);

        // Run game
        window.loop();
    } catch(...) {
        lau::lerr << "[error] Unhandled exception thrown." << endl;
    }

    return 0;
}
#endif
