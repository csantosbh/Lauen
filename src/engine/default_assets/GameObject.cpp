#include "GameObject.hpp"
#include "Component.hpp"
#include "Peekers.hpp"
#include "window/NaCl.hpp"

namespace lau {

extern int generateInstanceId();

GameObject::GameObject(const rapidjson::Value& serializedObject)
#ifdef PREVIEW_MODE
	: gameObjectId(generateInstanceId()), gameObjectName(serializedObject["name"].GetString())
#endif
	{
#ifdef PREVIEW_MODE
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	gameObjectInfo.Set("name", gameObjectName);
	NaCl::getInstance()->createGameObject(gameObjectInfo);
#endif
}

GameObject::~GameObject() {
#ifdef PREVIEW_MODE
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	NaCl::getInstance()->deleteGameObject(gameObjectInfo);
#endif
}

void GameObject::update(float dt) {
#ifdef PREVIEW_MODE
	pp::VarDictionary currentState;
	pp::VarArray componentStates;
	currentState.Set("instanceId", gameObjectId);
#endif

    for(auto& updateableComponent: this->updateableComponents) {
        updateableComponent->update(dt);
#ifdef PREVIEW_MODE
        updateableComponent->lau_peeker__->update();
		pp::VarDictionary componentState;
		componentState.Set("instanceId", updateableComponent->lau_peeker__->getInstanceId());
		componentState.Set("fields", updateableComponent->lau_peeker__->getCurrentState());
		componentStates.Set(componentStates.GetLength(), componentState);
#endif
    }

#ifdef PREVIEW_MODE
	currentState.Set("components", componentStates);
	NaCl::getInstance()->publishState(currentState);
#endif
}

void GameObject::draw(float alpha) {
    for(auto& drawableComponent: this->drawableComponents) {
        drawableComponent->draw(alpha);
    }
}

void GameObject::addComponent(const shared_ptr<Component>& component) {
    this->updateableComponents.push_back(component);

    // If component is drawable, add it to the drawable list
    if(dynamic_cast<DrawableComponent*>(component.get()) != nullptr) {
        this->drawableComponents.push_back(dynamic_pointer_cast<DrawableComponent>(component));
    }

#ifdef PREVIEW_MODE
	auto& peeker = component->lau_peeker__;
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	pp::VarDictionary componentInfo;
	componentInfo.Set("componentId", component->getId());
	componentInfo.Set("instanceId", peeker->getInstanceId());
	gameObjectInfo.Set("component", componentInfo);
	NaCl::getInstance()->addComponent(gameObjectInfo);
#endif
}

} // namespace lau
