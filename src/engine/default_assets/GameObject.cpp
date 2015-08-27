#include "GameObject.hpp"
#include "Component.hpp"
#include "Peekers.hpp"
#include "window/NaCl.hpp"
#include "Game.hpp" // TODO remove this from here after gameObjects_ are placed in the GameObject class

using namespace std;

namespace lau {

extern int generateInstanceId();

GameObject::GameObject(const rapidjson::Value& serializedObject, const GameObject* parent) :
    transform(serializedObject["transform"])
#ifdef PREVIEW_MODE
	, gameObjectId(generateInstanceId()), gameObjectName(serializedObject["name"].GetString())
#endif
	{
#ifdef PREVIEW_MODE
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	gameObjectInfo.Set("name", gameObjectName);
    gameObjectInfo.Set("transform", transform.getCurrentState());
    if(parent != nullptr)
        gameObjectInfo.Set("parentId", parent->gameObjectId);
    else
        gameObjectInfo.Set("parentId", pp::Var::Null());
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

    // Update components
    for(auto& updateableComponent: this->updateableComponents_) {
        updateableComponent->update(dt);
#ifdef PREVIEW_MODE
        updateableComponent->lau_peeker__->update();
		pp::VarDictionary componentState;
		componentState.Set("instanceId", updateableComponent->lau_peeker__->getInstanceId());
		componentState.Set("fields", updateableComponent->lau_peeker__->getCurrentState());
		componentStates.Set(componentStates.GetLength(), componentState);
#endif
    }

    // Update transform
    transform.update(dt);

    // Update child game objects
    for(auto& child: this->children_) {
        // TODO pass stacked transform
        child->update(dt);
    }

#ifdef PREVIEW_MODE
    currentState.Set("transform", transform.getCurrentState());
	currentState.Set("components", componentStates);
	NaCl::getInstance()->publishState(currentState);
#endif
}

void GameObject::draw(float alpha) {
    for(auto& drawableComponent: this->drawableComponents_) {
        drawableComponent->draw(alpha);
    }
}

void GameObject::addChild(const shared_ptr<GameObject>& gameObj) {
    this->children_.push_back(gameObj);
}

void GameObject::addComponent(const shared_ptr<Component>& component) {
    this->updateableComponents_.push_back(component);

    // If component is drawable, add it to the drawable list
    if(dynamic_cast<DrawableComponent*>(component.get()) != nullptr) {
        this->drawableComponents_.push_back(dynamic_pointer_cast<DrawableComponent>(component));
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

const vector<shared_ptr<GameObject>>& GameObject::allGameObjects() {
    return lau_internal::GameInstance->allGameObjects();
}

} // namespace lau
