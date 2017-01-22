#pragma once

namespace lau {

namespace math {

template<typename T>
T& map(const void* ptr) {
    return *(T*)ptr;
}

template<typename T>
T clamp(T value, T min, T max) {
    return value >= min ?
           (value <= max ? value : max) :
           min;
}

}
}
