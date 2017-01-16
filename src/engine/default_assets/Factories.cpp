#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "Factories.hpp"
#include "LauCommon.h"

#include "Component.hpp"

namespace lau {

std::map<int, std::shared_ptr<Component>(*)(shared_ptr<GameObject>&, const rapidjson::Value&)> Factories::componentInstanceFactories STATIC_INITIALIZER_GROUP_A;

shared_ptr<Component> Factories::componentFactory(shared_ptr<GameObject>& gameObj, const rapidjson::Value& serializedComponent) {
	if(serializedComponent.HasMember("id")) {
#ifdef DEBUG
		if(componentInstanceFactories.find(serializedComponent["id"].GetInt()) == componentInstanceFactories.end()) {
			lerr << "Could not find component of id " << serializedComponent["id"].GetInt() << endl;
		}
#endif
		return componentInstanceFactories[serializedComponent["id"].GetInt()](gameObj, serializedComponent["fields"]);
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	serializedComponent.Accept(writer);
	const char* str = buffer.GetString();
	lerr << "Serialized component has no <id>: " << buffer.GetString() << endl;
	return nullptr;
}

shared_ptr<GameObject> Factories::assembleGameObject(const rapidjson::Value& serializedObj, const GameObject* parent) {
    shared_ptr<GameObject> obj(new GameObject(serializedObj, parent));

    // Add components
    const rapidjson::Value& components = serializedObj["components"];
    for(int c = 0; c < components.Size(); ++c) {
        shared_ptr<Component> component = componentFactory(obj, components[c]);

        // TODO assert that component cant be null?
        if(component != NULL) {
            obj->addComponent(component);
        }
#ifdef DEBUG
        else {
            lerr << "[error] Could not create requested component: {id="<< components[c]["id"].GetInt() << ", type=" << components[c]["type"].GetString() <<"}" << endl;
        }
#endif
    }

    // Add children objects
    const rapidjson::Value& children = serializedObj["children"];
    for(int g = 0; g < children.Size(); ++g) {
        obj->addChild(assembleGameObject(children[g], obj.get()));
    }

    return obj;
}

vector<shared_ptr<GameObject>> Factories::gameObjectFactory(const rapidjson::Document& objects) {
	vector<shared_ptr<GameObject>> result;
	for(int i = 0; i < objects.Size(); ++i) {
		result.push_back(assembleGameObject(objects[i], nullptr));
	}

	return result;
}

} // namespace lau
