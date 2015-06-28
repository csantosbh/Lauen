#pragma once

#include <memory>
#include <vector>

#include <rapidjson/document.h>

#include "Component.hpp"
#include "GameObject.hpp"

// TODO figure out a way to move all the preview related code elsewhere
namespace lau {

using namespace std;

////
// Factories
////
class Factories {
public:
	static shared_ptr<Component> componentFactory(const rapidjson::Value& serializedComponent);

	static vector<shared_ptr<GameObject>> gameObjectFactory(const rapidjson::Document& objects);

};

}
