#pragma once

#include <rapidjson/document.h>

#include "Component.hpp"

namespace lau {

using namespace std;
class Transform : public Component {
public:
	Transform(const rapidjson::Value& fields) {}
	virtual void update(float dt) {
		cout << "Transform update!" << endl;
	}
};

} // namespace
