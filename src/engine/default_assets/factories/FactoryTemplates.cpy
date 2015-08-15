// Do NOT edit!
// Automatically generated!

// Whenever changing these includes, make sure to update the dependencies of
// the UserFactoriesProcessor class in server/project/AssetProcessor.py
// accordingly.
#include "default_assets/Factories.hpp"
#include "default_assets/Peekers.hpp"
#include "default_assets/LauCommon.h"
#include "${component['path']}"

namespace lau {

% if 'namespace' in component and 'class' in component:

#ifdef PREVIEW_MODE
/////////
// Script component peekers
/////
template<>
ComponentPeekerImpl<${component['namespace']}::${component['class']}>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<${component['namespace']}::${component['class']}>(actualComponent)) {
}

template<>
void ComponentPeekerImpl<${component['namespace']}::${component['class']}>::update() {
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
}
#endif

template<>
int Component::getComponentId<${component['namespace']}::${component['class']}>() {
	return ${component['id']};
}

template<>
shared_ptr<Component> Factories::componentInternalFactory<${component['namespace']}::${component['class']}>(shared_ptr<GameObject>&, const rapidjson::Value& fields) {
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

	shared_ptr<Component> result;
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#ifdef PREVIEW_MODE
	result->lau_peeker__ = shared_ptr<ComponentPeeker>(dynamic_cast<ComponentPeeker*>(new ComponentPeekerImpl<${component['namespace']}::${component['class']}>(result)));
#endif

	result->setId(${component['id']});
	return result;
}

template<>
struct Initializer<${component['namespace']}::${component['class']}> {
	Initializer() {
		Factories::componentInstanceFactories[${component['id']}] = &Factories::componentInternalFactory<${component['namespace']}::${component['class']}>;
	}
	static Initializer<${component['namespace']}::${component['class']}> instance;
};
Initializer<${component['namespace']}::${component['class']}> Initializer<${component['namespace']}::${component['class']}>::instance STATIC_INITIALIZER_GROUP_C;

% endif

} // namespace lau
