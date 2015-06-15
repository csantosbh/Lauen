#pragma once

#include <rapidjson/document.h>
#ifndef NACL
#include <iostream>
#endif

#include "Component.hpp"

namespace lau {

using namespace std;
class Transform : public Component {
public:
	Transform(const rapidjson::Value& fields) {}
	virtual void update(float dt) {
#ifndef NACL
		cout << "Transform update!" << endl;
#endif
	}
};

} // namespace
