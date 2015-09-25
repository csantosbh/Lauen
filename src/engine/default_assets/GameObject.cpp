#include "GameObject.hpp"
#include "Factories.hpp"
#include "Component.hpp"
#include "Peekers.hpp"
#include "window/NaCl.hpp"

using namespace std;

namespace lau {

std::vector<std::shared_ptr<GameObject>> GameObject::gameObjects_;

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

    handleRequestedNewComponents();
    handleRequestedDestroyedComponents();

    // Update transform
    transform.update(dt);
	transform.updateObject2World();

    // Update child game objects
    for(auto& child: this->children_) {
        child->update(this, dt);
    }

#ifdef PREVIEW_MODE
    currentState.Set("transform", transform.getCurrentState());
	currentState.Set("components", componentStates);
	NaCl::getInstance()->publishState(currentState);
#endif
}

// TODO figure out a way to make this function reuse code from the regular update. They are nearly identical.
void GameObject::update(GameObject* parent, float dt) {
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

    handleRequestedNewComponents();
    handleRequestedDestroyedComponents();

    // Update transform
    transform.update(dt);
	transform.updateObject2World(parent->transform.getObject2WorldMatrix(), parent->transform.getObject2WorldTranspOfInvMatrix());

    // Update child game objects
    for(auto& child: this->children_) {
        child->update(this, dt);
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

    // Draw child game objects
    for(auto& child: this->children_) {
        child->draw(alpha);
    }
}

void GameObject::addChild(const shared_ptr<GameObject>& gameObj) {
    this->children_.push_back(gameObj);
}

void GameObject::addComponent(const shared_ptr<Component>& component) {
    component->setGameObject(this);
    this->updateableComponents_.push_back(component);

    // If component is drawable, add it to the drawable list
    if(dynamic_cast<DrawableComponent*>(component.get()) != nullptr) {
        this->drawableComponents_.insert(dynamic_cast<DrawableComponent*>(component.get()));
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
    return gameObjects_;
}

void GameObject::handleRequestedNewComponents() {
    while(!addComponentRequested_.empty()) {
        auto& component = addComponentRequested_.front();
        addComponent(component);
        addComponentRequested_.pop_front();
    }
}

void GameObject::destroyComponent(const Component* comp) {
    delComponentRequested_.push_back(comp);
}

void GameObject::handleRequestedDestroyedComponents() {
    for(auto& comp: delComponentRequested_) {
        int pos = getComponentByInstanceId(comp->getId());

        assert(pos>=0);

        const DrawableComponent* component = dynamic_cast<const DrawableComponent*>(comp);
        if(component != nullptr)
            drawableComponents_.erase(const_cast<DrawableComponent*>(component));

#ifdef PREVIEW_MODE
        pp::VarDictionary destroyedComponentInfo;
        destroyedComponentInfo.Set("gameObjectId", gameObjectId);
        destroyedComponentInfo.Set("componentId", comp->lau_peeker__->getInstanceId());
        NaCl::getInstance()->deleteComponent(destroyedComponentInfo);
#endif

        updateableComponents_.erase(updateableComponents_.begin() + pos);
    }
    delComponentRequested_.clear();
}

int GameObject::getComponentByInstanceId(int id) {
    for(int i = 0; i < updateableComponents_.size(); ++i)
        if(updateableComponents_[i]->getId() == id)
            return i;
    return -1;
}

void GameObject::instantiateScene(const rapidjson::Document& serializedGameObjs) {
    gameObjects_ = Factories::gameObjectFactory(serializedGameObjs);
}

void GameObject::clearScene() {
    gameObjects_.clear();
}

} // namespace lau
