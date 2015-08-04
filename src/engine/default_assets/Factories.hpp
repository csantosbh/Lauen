#pragma once

#include <memory>
#include <vector>
#include <map>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "Component.hpp"
#include "GameObject.hpp"

#include "Peekers.hpp"

namespace lau {

using namespace std;

////
// Factories
////
class Factories {
public:
	static shared_ptr<Component> componentFactory(shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent);
	static vector<shared_ptr<GameObject>> gameObjectFactory(const rapidjson::Document& objects);
	static std::map<int, std::shared_ptr<Component>(*)(shared_ptr<GameObject>&, const rapidjson::Value&)> componentInstanceFactories;

	template<class T>
	static shared_ptr<Component> componentInternalFactory(shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent);
};

template<typename T>
struct Initializer {
	Initializer();
	static Initializer<T> instance;
};

}
