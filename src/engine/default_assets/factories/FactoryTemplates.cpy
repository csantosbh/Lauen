// Do NOT edit!
// Automatically generated!

#include "../Factories.hpp"
#include "../Peekers.hpp"
#include "../LauCommon.h"
#include "${component['path']}"

namespace lau {

% if 'namespace' in component and 'class' in component:

#ifdef PREVIEW_MODE
/////////
// Script component peekers
/////
template<>
ComponentPeeker<${component['namespace']}::${component['class']}>::ComponentPeeker(shared_ptr<${component['namespace']}::${component['class']}> actualComp) : impl(actualComp)
{ }

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
#ifndef PREVIEW_MODE
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
	result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<${component['namespace']}::${component['class']}>(shared_ptr<${component['namespace']}::${component['class']}>(ptr))));
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
STATIC_INITIALIZER_GROUP_C(Initializer<${component['namespace']}::${component['class']}> Initializer<${component['namespace']}::${component['class']}>::instance);

% endif

} // namespace lau
