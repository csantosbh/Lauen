#include "Factories.hpp"
#include "Light.hpp"
#include "math/Vector.hpp"

using namespace std;

using namespace rapidjson;
using namespace lau::math;

namespace lau {

std::set<Light*> Light::lights_ STATIC_INITIALIZER_GROUP_A;
const Light* Light::current = nullptr;

Light::Light(const rapidjson::Value& fields) {
    color = Color(fields["color"].GetUint());
    // Add this light to the game's light collection
    lights_.insert(this);
}

Light::~Light() {
    // Make sure the game will forget this light
    lights_.erase(this);
}

void Light::draw(float temporalAlpha) {
    // TODO implement light halo/lens flare
}

const vector<float> Light::allLightPositions() {
    vector<float> result;
    result.reserve(3*lights_.size());

    for(const auto light: lights_) {
        auto& lightTransformObj = light->gameObject->transform;
        auto light2world = lightTransformObj.getObject2WorldMatrix();
        auto lightPosition = light2world *
            vec4(lightTransformObj.position[0],
                 lightTransformObj.position[0],
                 lightTransformObj.position[0], 1.0f);
        for(int i = 0; i < 3; ++i) {
            result.push_back(lightPosition[i]);
        }
    }

    return result;
}

const vector<float> Light::allLightColors() {
    vector<float> result;
    result.reserve(4*lights_.size());

    for(const auto light: lights_) {
        auto color = light->color.getRgbaV4f();
        for(int i = 0; i < 4; ++i) {
            result.push_back(color[i]);
        }
    }

    return result;
}

//////
// Factory
#define LIGHT_ID 3
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Light, LIGHT_ID)

} // namespace lau

