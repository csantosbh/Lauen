#include "Factories.hpp"
#include "Transform.hpp"
#include "LauCommon.h"

namespace lau {

Transform::Transform(const rapidjson::Value& fields) : rotation(Eigen::Quaternionf::Identity()) {
    const auto& pos = fields["position"];
    const auto& rot = fields["rotation"];
    const auto& scale = fields["scale"];
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

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

void Transform::update(float dt) {
    rotation = rotation*Eigen::AngleAxisf(0.1 * M_PI/180.0f, Eigen::Vector3f::UnitZ());

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

#ifdef PREVIEW_MODE
void Transform::serializeState() {
	// TODO figure out how the eulerangles are being returned (in which order?), and make sure it is consistent with the order in the Editor
	Eigen::Vector3f rotation = this->rotation.toRotationMatrix().eulerAngles(0, 1, 2);

	pp::VarArray pos;
	pp::VarArray rot;
	pp::VarArray scale;
	for(int i = 0; i < 3; ++i) {
		pos.Set(i, this->position[i]);
		rot.Set(i, rotation[i]);
		scale.Set(i, this->scale[i]);
	}

	currentState.Set("position", pos);
	currentState.Set("scale", scale);
	currentState.Set("rotation", rot);
}
#endif

} // namespace lau
