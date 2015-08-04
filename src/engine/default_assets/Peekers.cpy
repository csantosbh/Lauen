// TODO make this a regular cpp file

#include "Peekers.hpp"
% for type,default_component in default_components.iteritems():
#include "${default_component['path']}"
% endfor

#ifdef PREVIEW_MODE

namespace lau {

int lastInstanceId = 0;
int generateInstanceId() {
	// TODO this is not thread safe
	return lastInstanceId++;
}


/////////
// Standard component peekers
/////

///// Transform
template<>
ComponentPeeker<Transform>::ComponentPeeker(shared_ptr<Transform> actualComp) : impl(actualComp)
{ }

template<>
const pp::VarDictionary& ComponentPeeker<Transform>::getCurrentState() {
	// TODO figure out how the eulerangles are being returned (in which order?), and make sure it is consistent with the order in the Editor
	Eigen::Vector3f rotation = impl->rotation.toRotationMatrix().eulerAngles(0, 1, 2);

	pp::VarArray pos;
	pp::VarArray rot;
	pp::VarArray scale;
	for(int i = 0; i < 3; ++i) {
		pos.Set(i, impl->position[i]);
		rot.Set(i, rotation[i]);
		scale.Set(i, impl->scale[i]);
	}

	currentState.Set("position", pos);
	currentState.Set("scale", scale);
	currentState.Set("rotation", rot);

	return currentState;
}

///// Mesh
template<>
ComponentPeeker<Mesh>::ComponentPeeker(shared_ptr<Mesh> actualComp) : impl(actualComp)
{ }

template<>
const pp::VarDictionary& ComponentPeeker<Mesh>::getCurrentState() {
	currentState.Set("mesh", "tmp");

	return currentState;
}

///// MeshRenderer
template<>
ComponentPeeker<MeshRenderer>::ComponentPeeker(shared_ptr<MeshRenderer> actualComp) : impl(actualComp)
{ }

template<>
const pp::VarDictionary& ComponentPeeker<MeshRenderer>::getCurrentState() {
	return currentState;
}

/////////
// Game object peeker
/////

GameObjectPeeker::GameObjectPeeker(const rapidjson::Value& serializedObject) : gameObjectId(generateInstanceId()), gameObjectName(serializedObject["name"].GetString()) {
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	gameObjectInfo.Set("name", gameObjectName);
	NaCl::getInstance()->createGameObject(gameObjectInfo);
}

GameObjectPeeker::~GameObjectPeeker() {
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	NaCl::getInstance()->deleteGameObject(gameObjectInfo);
}

void GameObjectPeeker::update(float dt) {
	pp::VarDictionary currentState;
	pp::VarArray componentStates;
	currentState.Set("instanceId", gameObjectId);

	for(auto& updateableComponent: this->updateableComponents) {
		auto compPeeker = std::dynamic_pointer_cast<ComponentPeekerBase>(updateableComponent);
		compPeeker->update(dt);
		pp::VarDictionary componentState;
		componentState.Set("instanceId", compPeeker->getInstanceId());
		componentState.Set("fields", compPeeker->getCurrentState());
		componentStates.Set(componentStates.GetLength(), componentState);
	}
	currentState.Set("components", componentStates);
	NaCl::getInstance()->publishState(currentState);
}

void GameObjectPeeker::addComponent(const shared_ptr<Component>& actualComp) {
	auto compWrapper = std::dynamic_pointer_cast<ComponentPeekerBase>(actualComp);
	this->updateableComponents.push_back(compWrapper);

	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
	pp::VarDictionary componentInfo;
	componentInfo.Set("componentId", compWrapper->getId());
	componentInfo.Set("instanceId", compWrapper->getInstanceId());
	gameObjectInfo.Set("component", componentInfo);
	NaCl::getInstance()->addComponent(gameObjectInfo);
}

} // namespace lau
#endif
