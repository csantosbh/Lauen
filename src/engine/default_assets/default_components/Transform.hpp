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
	}
	void update(float dt) {
		rotation = rotation*Eigen::AngleAxisf(0.1 * M_PI/180.0f, Eigen::Vector3f::UnitZ());
    }
};

} // namespace
