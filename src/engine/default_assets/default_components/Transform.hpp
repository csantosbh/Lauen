#pragma once

#include <Eigen>
#include <rapidjson/document.h>

#include "Component.hpp"

namespace lau {

using namespace std;
class Transform : public Component {
public:
	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;
	Transform(const rapidjson::Value& fields);

	void update(float dt);
};

} // namespace
