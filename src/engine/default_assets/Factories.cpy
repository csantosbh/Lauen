// Do NOT edit!
// Automatically generated!

#include <Eigen>

#include "Factories.hpp"

% for type,default_component in default_components.iteritems():
#include "${default_component['path']}"
% endfor
% for component in components:
#include "${component['path']}"
% endfor

#ifdef NACL
#include "window/NaCl.hpp"
#endif

<%
	#///
	#// Python helper functions
	#///

	def isVecType(type):
		return type == 'v4f' or type == 'v3f' or type == 'v2f'
		pass

	vec_iterations=dict(v4f=4, v3f=3, v2f=2)
%>

namespace lau {
////
// Component Peekers
////
#ifdef PREVIEW_MODE
class ComponentPeeker: public Component {
public:
	ComponentPeeker() : instanceId(ComponentPeeker::generateInstanceId())
	{}
	virtual const pp::VarDictionary& getCurrentState() = 0;
	virtual int getComponentId() = 0;
	int getInstanceId() {
		return instanceId;
	}
	virtual ~ComponentPeeker() {}
protected:
	pp::VarDictionary currentState;
	int instanceId;

	static int generateInstanceId() {
		// TODO this is not thread safe
		return lastComponentId++;
	}
	static int lastComponentId;
};
int ComponentPeeker::lastComponentId = 0;

// TODO figure out a way to create a separate cpp file for peekers
template<typename T>
class ComponentPeekerImpl : public ComponentPeeker {};

% for component in components:
template<>
class ComponentPeekerImpl<${component['namespace']}::${component['class']}> : public ComponentPeeker
{
public:
	ComponentPeekerImpl(shared_ptr<${component['namespace']}::${component['class']}> actualComp) : impl(actualComp) {
	}

	int getComponentId() {
		return ${component['id']};
	}

	const pp::VarDictionary& getCurrentState() {
		// Peek class fields
		% for f in component['fields']:
			% if isVecType(component['types'][f]):
		{
		pp::VarArray vec;
		for(int i = 0; i < ${vec_iterations[component['types'][f]]}; ++i) {
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

	void update(float dt) {
		impl->update(dt);
	}

private:
    shared_ptr<${component['namespace']}::${component['class']}> impl; // Pointer to the actual component
};
% endfor

class GameObjectPeeker : public GameObject {
public:
	GameObjectPeeker() : gameObjectId(generateInstanceId()) {
		pp::VarDictionary gameObjectInfo;
		gameObjectInfo.Set("instanceId", gameObjectId);
		NaCl::getInstance()->createGameObject(gameObjectInfo);
	}
	~GameObjectPeeker() {
		pp::VarDictionary gameObjectInfo;
		gameObjectInfo.Set("instanceId", gameObjectId);
		NaCl::getInstance()->deleteGameObject(gameObjectInfo);
	}

	void update(float dt) {
		pp::VarDictionary currentState;
		pp::VarArray componentStates;
		currentState.Set("instanceId", gameObjectId);

        for(auto& updateableComponent: this->updateableComponents) {
			auto compPeeker = std::dynamic_pointer_cast<ComponentPeeker>(updateableComponent);
            compPeeker->update(dt);
			pp::VarDictionary componentState;
			componentState.Set("instanceId", compPeeker->getInstanceId());
			componentState.Set("fields", compPeeker->getCurrentState());
			componentStates.Set(componentStates.GetLength(), componentState);
        }
		currentState.Set("components", componentStates);
		NaCl::getInstance()->publishState(currentState);
	}

	void addComponent(const shared_ptr<Component>& actualComp) {
		auto compWrapper = std::dynamic_pointer_cast<ComponentPeeker>(actualComp);
		this->updateableComponents.push_back(compWrapper);

		pp::VarDictionary gameObjectInfo;
		gameObjectInfo.Set("instanceId", gameObjectId);
		pp::VarDictionary componentInfo;
		componentInfo.Set("componentId", compWrapper->getComponentId());
		componentInfo.Set("instanceId", compWrapper->getInstanceId());
		gameObjectInfo.Set("component", componentInfo);
		NaCl::getInstance()->addComponent(gameObjectInfo);
	}

private:
	static int generateInstanceId() {
		// TODO this is not thread safe
		return lastGameObjectId++;
	}
	static int lastGameObjectId;
	int gameObjectId;
};
int GameObjectPeeker::lastGameObjectId = 0;

///// Transform
template<>
class ComponentPeekerImpl<Transform> : public ComponentPeeker
{
public:
	ComponentPeekerImpl(shared_ptr<Transform> actualComp) : impl(actualComp) {
	}

	void update(float dt) {
		impl->update(dt);
	}

	const pp::VarDictionary& getCurrentState() {
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

	int getComponentId() {
		return ${default_components['transform']['id']};
	}

private:
	shared_ptr<Transform> impl;
};
#endif // PREVIEW_MODE

////
// Factories
////
shared_ptr<Component> Factories::componentFactory(const rapidjson::Value& serializedComponent) {
	shared_ptr<Component> result;

	// TODO: make sure all components have an id member
	if(serializedComponent.HasMember("id")) {
		switch(serializedComponent["id"].GetInt()) {
			// Default components
			% for type, default_component in default_components.iteritems():
			case ${default_component['id']}: {
				const rapidjson::Value& fields = serializedComponent["fields"];

				${default_component['full_class_name']}* ptr = new ${default_component['full_class_name']}(fields);
#ifndef PREVIEW_MODE
				result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
				result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeekerImpl<${default_component['full_class_name']}>(shared_ptr<${default_component['full_class_name']}>(ptr))));
#endif
				break;
			}
			% endfor

			// User scripts
			% for component in components:
			case ${component['id']}: {
				const rapidjson::Value& fields = serializedComponent["fields"];

				${component['namespace']}::${component['class']}* ptr = new ${component['namespace']}::${component['class']}();

				% for f in component['fields']:
					% if component['types'][f] == 'float' or component['types'][f] == 'double':
				ptr->${f} = fields["${f}"].GetDouble();
					% elif isVecType(component['types'][f]):
				{
				const auto& vec = fields["${f}"];
				for(int i = 0; i < ${vec_iterations[component['types'][f]]}; ++i) {
					ptr->${f}[i] = vec[i].GetDouble();
				}
				}
					% endif
				% endfor

#ifndef PREVIEW_MODE
				result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
				result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeekerImpl<${component['namespace']}::${component['class']}>(shared_ptr<${component['namespace']}::${component['class']}>(ptr))));
#endif
				break;
			}
			% endfor
		}
	}

	return result;
}

vector<shared_ptr<GameObject>> Factories::gameObjectFactory(const rapidjson::Document& objects) {
	vector<shared_ptr<GameObject>> result;
	for(int i = 0; i < objects.Size(); ++i) {
#ifndef PREVIEW_MODE
		shared_ptr<GameObject> obj(new GameObject());
#else
		shared_ptr<GameObject> obj(new GameObjectPeeker());
#endif
		const rapidjson::Value& components = objects[i]["components"];

		for(int c = 0; c < components.Size(); ++c) {
			shared_ptr<Component> component = componentFactory(components[c]);

			// TODO assert that component cant be null?
			if(component != NULL) {
#ifndef PREVIEW_MODE
				obj->addComponent(component);
#else
				obj->addComponent(component);
#endif
			}
		}

		result.push_back(obj);
	}

	return result;
}

} // namespace lau
