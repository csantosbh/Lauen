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

namespace lau {
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
				result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<${default_component['full_class_name']}>(shared_ptr<${default_component['full_class_name']}>(ptr))));
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
				for(int i = 0; i < ${vecIterations[component['types'][f]]}; ++i) {
					ptr->${f}[i] = vec[i].GetDouble();
				}
				}
					% endif
				% endfor

#ifndef PREVIEW_MODE
				result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
				result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<${component['namespace']}::${component['class']}>(shared_ptr<${component['namespace']}::${component['class']}>(ptr))));
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
