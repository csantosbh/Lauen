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

////
// Factories
////
class Factories {
public:
	static std::shared_ptr<Component> componentFactory(std::shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent);
	static std::vector<std::shared_ptr<GameObject>> gameObjectFactory(const rapidjson::Document& objects);
	static std::map<int, std::shared_ptr<Component>(*)(std::shared_ptr<GameObject>&, const rapidjson::Value&)> componentInstanceFactories;

	template<class T>
	static std::shared_ptr<Component> componentInternalFactory(std::shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent);

private:
    static std::shared_ptr<GameObject> assembleGameObject(const rapidjson::Value& serializedObj);
};

template<typename T>
struct Initializer {
	Initializer();
	static Initializer<T> instance;
};

}
