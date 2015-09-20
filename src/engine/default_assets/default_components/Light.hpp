#pragma once

#include <set>

#include <rapidjson/document.h>
#include "DrawableComponent.hpp"
#include "types/Color.hpp"

namespace lau {

class Light : public DrawableComponent {
public:
    Light(const rapidjson::Value& fields);
    ~Light();

    static const std::set<Light*>& allLights() {
        return lights_;
    }
    static const Light* current;

    void draw(float temporalAlpha);
    Color color;

private:
    static std::set<Light*> lights_;
};

} // namespace lau
