#include <Eigen>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "Factories.hpp"
#include "LauCommon.h"

#include "default_components/MeshRenderer.hpp"
#include "default_components/Mesh.hpp"
#include "default_components/Transform.hpp"
#include "Component.hpp"


namespace lau {

std::map<int, std::shared_ptr<Component>(*)(shared_ptr<GameObject>&, const rapidjson::Value&)> Factories::componentInstanceFactories STATIC_INITIALIZER_GROUP_A;

shared_ptr<Component> Factories::componentFactory(shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent) {
	if(serializedComponent.HasMember("id")) {
#ifdef DEBUG
		if(componentInstanceFactories.find(serializedComponent["id"].GetInt()) == componentInstanceFactories.end()) {
			// TODO change this to lerr once it is working with the editor
			lout << "Could not find component of id " << serializedComponent["id"].GetInt() << endl;
		}
#endif
		return componentInstanceFactories[serializedComponent["id"].GetInt()](gameObj, serializedComponent["fields"]);
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	serializedComponent.Accept(writer);
	const char* str = buffer.GetString();
	// TODO URGENT lerr is not being redirected to the editor in x86_linux mode
	// TODO change this to lerr once it is working with the editor
	lout << "Serialized component has no <id>: " << buffer.GetString() << endl;
	return nullptr;
}

vector<shared_ptr<GameObject>> Factories::gameObjectFactory(const rapidjson::Document& objects) {
	vector<shared_ptr<GameObject>> result;
	for(int i = 0; i < objects.Size(); ++i) {
		shared_ptr<GameObject> obj(new GameObject(objects[i]));
		const rapidjson::Value& components = objects[i]["components"];

		for(int c = 0; c < components.Size(); ++c) {
			shared_ptr<Component> component = componentFactory(obj, components[c]);

			// TODO assert that component cant be null?
			if(component != NULL) {
				component->setGameObject(obj);
				obj->addComponent(component);
			}
		}

		result.push_back(obj);
	}

	return result;
}

} // namespace lau
