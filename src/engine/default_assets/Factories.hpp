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

	template<class CompType, int CompId>
	static std::shared_ptr<Component> componentInternalFactory(std::shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent) {
		CompType* ptr = new CompType(serializedComponent);

		std::shared_ptr<Component> result(dynamic_cast<Component*>(ptr));
#ifdef PREVIEW_MODE
		result->lau_peeker__ = std::shared_ptr<ComponentPeeker>(dynamic_cast<ComponentPeeker*>(new ComponentPeekerImpl<CompType>(result)));
#endif

		result->setId(CompId);

		return result;
	}

private:
    static std::shared_ptr<GameObject> assembleGameObject(const rapidjson::Value& serializedObj, const GameObject* parent);
};

template<typename ComponentType, int componentId>
struct Initializer {
	Initializer() {
		Factories::componentInstanceFactories[componentId] = &Factories::componentInternalFactory<ComponentType, componentId>;
	}
	static Initializer<ComponentType, componentId> instance;
};

#define __LAU_CREATE_COMPONENT_INITIALIZER(ComponentType, ComponentId) \
template<>\
int Component::getComponentId<ComponentType>() {\
	return ComponentId;\
}\
template<typename T, int id>\
Initializer<T, id> Initializer<T, id>::instance;\
template struct Initializer<ComponentType, ComponentId>;

}
