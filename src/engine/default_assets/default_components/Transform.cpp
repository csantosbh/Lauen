#include "Factories.hpp"
#include "Transform.hpp"

namespace lau {

Transform::Transform(const rapidjson::Value& fields) : rotation(Eigen::Quaternionf::Identity()) {
    const auto& pos = fields["position"];
    const auto& rot = fields["scale"];
    const auto& scale = fields["rotation"];
    float eulerAngles[3];

    for(int i = 0; i < 3; ++i) {
        this->position[i] = pos[i].GetDouble();
        eulerAngles[i] = rot[i].GetDouble();
        this->scale[i] = scale[i].GetDouble();
    }

    rotation = Eigen::Quaternionf(
            Eigen::AngleAxisf(eulerAngles[0], Eigen::Vector3f::UnitX()) *
            Eigen::AngleAxisf(eulerAngles[1], Eigen::Vector3f::UnitY()) *
            Eigen::AngleAxisf(eulerAngles[2], Eigen::Vector3f::UnitZ())
            );
}

void Transform::update(float dt) {
    rotation = rotation*Eigen::AngleAxisf(0.1 * M_PI/180.0f, Eigen::Vector3f::UnitZ());
}

////
// Factories
#define TRANSFORM_ID 0
template<>
int Component::getComponentId<lau::Transform>() {
	return TRANSFORM_ID;
}

template<>
shared_ptr<Component> Factories::componentInternalFactory<lau::Transform>(shared_ptr<GameObject>& gameObj, const rapidjson::Value& fields) {
	lau::Transform* ptr = new lau::Transform(fields);

	shared_ptr<Component> result;
#ifndef PREVIEW_MODE
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
	result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<lau::Transform>(shared_ptr<lau::Transform>(ptr))));
#endif

	result->setId(TRANSFORM_ID);

	return result;
}

template<>
struct Initializer<lau::Transform> {
	Initializer() {
		Factories::componentInstanceFactories[TRANSFORM_ID] = &Factories::componentInternalFactory<lau::Transform>;
	}
	static Initializer<lau::Transform> instance;
};
Initializer<lau::Transform> Initializer<lau::Transform>::instance;

} // namespace lau
