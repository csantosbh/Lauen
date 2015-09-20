#include "Factories.hpp"
#include "Light.hpp"

using namespace std;

using namespace rapidjson;
using namespace Eigen;

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

//////
// Factory
#define LIGHT_ID 3
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Light, LIGHT_ID)

} // namespace lau

