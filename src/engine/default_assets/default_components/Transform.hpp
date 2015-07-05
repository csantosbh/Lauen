#pragma once

#include <Eigen>
#include <rapidjson/document.h>
#include <iostream>

#include "Component.hpp"

namespace lau {

using namespace std;
class Transform : public Component {
public:
	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;
	Transform(const rapidjson::Value& fields) : rotation(Eigen::Quaternionf::Identity()) {
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

	void update(float dt) {
		rotation = rotation*Eigen::AngleAxisf(0.1 * M_PI/180.0f, Eigen::Vector3f::UnitZ());
    }
};

} // namespace
