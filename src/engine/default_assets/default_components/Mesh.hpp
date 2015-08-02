#pragma once

#include <rapidjson/document.h>
#include <iostream>

#include "LauCommon.h"
#include "Component.hpp"

namespace lau {

using namespace std;
class Mesh : public Component {
public:
	Mesh(const rapidjson::Value& fields) {
        // TODO create actual mesh
        lout << fields["mesh"] << endl;
	}

	void update(float dt) {
    }
};

} // namespace
