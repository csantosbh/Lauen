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
    // TODO this function relies on the parent2world transform of each light. How can I make sure that the parent2world was properly initialized when this function is called?
    static const std::vector<float> allLightPositions();
    static const std::vector<float> allLightColors();

    static const Light* current;

    void draw(float temporalAlpha);
    Color color;

private:
    static std::set<Light*> lights_;
};

} // namespace lau
