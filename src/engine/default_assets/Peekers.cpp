// TODO make this a regular cpp file

#include "Peekers.hpp"
#include "default_components/MeshRenderer.hpp"
#include "default_components/Mesh.hpp"
#include "default_components/Transform.hpp"

#ifdef PREVIEW_MODE
#include "ppapi/cpp/var_array.h"

namespace lau {

int lastInstanceId = 0;
int generateInstanceId() {
	// TODO this is not thread safe
	return lastInstanceId++;
}

/////////
// Standard component peekers
/////

///// Mesh
template<>
ComponentPeekerImpl<Mesh>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<Mesh>(actualComponent)) {
    // TODO set current state here
	currentState.Set("mesh", "TODO");
}

template<>
void ComponentPeekerImpl<Mesh>::update() {
}

///// MeshRenderer
template<>
ComponentPeekerImpl<MeshRenderer>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<MeshRenderer>(actualComponent)) {
}

template<>
void ComponentPeekerImpl<MeshRenderer>::update() {
}

} // namespace lau
#endif
