#pragma once
#include "math/Vector.hpp"

namespace lau {

class mat3 {
public:
    mat3(){}
    mat3& operator=(const mat3& m);
    union {
        vec3 rows[3];
        float data[3*3];
    };
};

} // namespace lau
