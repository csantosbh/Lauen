// Do NOT edit!
// Automatically generated!

#include <Eigen>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

// TODO make this a regular cpp file
#include "Factories.hpp"

% for type,default_component in default_components.iteritems():
#include "${default_component['path']}"
% endfor


namespace lau {

////
// Factories
////
std::map<int, std::shared_ptr<Component>(*)(const rapidjson::Value&)> Factories::componentInstanceFactories;

% for type, default_component in default_components.iteritems():
template<>
shared_ptr<Component> Factories::componentInternalFactory<${default_component['full_class_name']}>(const rapidjson::Value& fields) {
	${default_component['full_class_name']}* ptr = new ${default_component['full_class_name']}(fields);

	shared_ptr<Component> result;
#ifndef PREVIEW_MODE
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
	result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<${default_component['full_class_name']}>(shared_ptr<${default_component['full_class_name']}>(ptr))));
#endif

	return result;
}

template<>
struct Initializer<${default_component['full_class_name']}> {
	Initializer() {
		Factories::componentInstanceFactories[${default_component['id']}] = &Factories::componentInternalFactory<${default_component['full_class_name']}>;
	}
	static Initializer<${default_component['full_class_name']}> instance;
};
Initializer<${default_component['full_class_name']}> Initializer<${default_component['full_class_name']}>::instance;

% endfor

shared_ptr<Component> Factories::componentFactory(const rapidjson::Value& serializedComponent) {
	if(serializedComponent.HasMember("id")) {
#ifdef DEBUG
		if(componentInstanceFactories.find(serializedComponent["id"].GetInt()) == componentInstanceFactories.end()) {
			// TODO change this to lerr once it is working with the editor
			lout << "Could not find component of id " << serializedComponent["id"].GetInt() << endl;
		}
#endif
		return componentInstanceFactories[serializedComponent["id"].GetInt()](serializedComponent["fields"]);
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
