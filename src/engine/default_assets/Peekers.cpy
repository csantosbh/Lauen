#ifdef PREVIEW_MODE

#include "Peekers.hpp"
% for component in components:
#include "${component['path']}"
% endfor
% for type,default_component in default_components.iteritems():
#include "${default_component['path']}"
% endfor

namespace lau {

int lastInstanceId = 0;
int generateInstanceId() {
	// TODO this is not thread safe
	return lastInstanceId++;
}

/////////
// Script component peekers
/////
% for component in components:
template<>
ComponentPeeker<${component['namespace']}::${component['class']}>::ComponentPeeker(shared_ptr<${component['namespace']}::${component['class']}> actualComp) : impl(actualComp)
{ }

template<>
int ComponentPeeker<${component['namespace']}::${component['class']}>::getComponentId() {
	return ${component['id']};
}

template<>
const pp::VarDictionary& ComponentPeeker<${component['namespace']}::${component['class']}>::getCurrentState() {
	// Peek class fields
	% for f in component['fields']:
		% if isVecType(component['types'][f]):
	{
		pp::VarArray vec;
		for(int i = 0; i < ${vecIterations[component['types'][f]]}; ++i) {
			vec.Set(i, impl->${f}[i]);
		}
		currentState.Set("${f}", vec);
	}
		% else: # all remaining components
		currentState.Set("${f}", impl->${f});
		% endif
	% endfor

	return currentState;
}

template<>
void ComponentPeeker<${component['namespace']}::${component['class']}>::update(float dt) {
	impl->update(dt);
}

% endfor

/////////
// Standard component peekers
/////

///// Transform
template<>
ComponentPeeker<Transform>::ComponentPeeker(shared_ptr<Transform> actualComp) : impl(actualComp)
{ }

template<>
void ComponentPeeker<Transform>::update(float dt) {
	impl->update(dt);
}

template<>
const pp::VarDictionary& ComponentPeeker<Transform>::getCurrentState() {
	// TODO make standard components have their stuff initialized with python scripting, just like user scripts
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

template<>
int ComponentPeeker<Transform>::getComponentId() {
	return ${default_components['transform']['id']};
}

/////////
// Game object peeker
/////

GameObjectPeeker::GameObjectPeeker() : gameObjectId(generateInstanceId()) {
	pp::VarDictionary gameObjectInfo;
	gameObjectInfo.Set("instanceId", gameObjectId);
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
	componentInfo.Set("componentId", compWrapper->getComponentId());
	componentInfo.Set("instanceId", compWrapper->getInstanceId());
	gameObjectInfo.Set("component", componentInfo);
	NaCl::getInstance()->addComponent(gameObjectInfo);
}

} // namespace lau
#endif
