#include <sstream>

#include "Peekers.hpp"
#include "default_components/Camera.hpp"
#include "default_components/MeshRenderer.hpp"
#include "default_components/Mesh.hpp"
#include "default_components/Light.hpp"

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

///// Camera
template<>
ComponentPeekerImpl<Camera>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<Camera>(actualComponent)) {
}

template<>
void ComponentPeekerImpl<Camera>::update() {
	currentState_.Set("near", impl->getNearPlane());
	currentState_.Set("far", impl->getFarPlane());
	currentState_.Set("width", impl->getNearPlaneWidth());
	currentState_.Set("priority", impl->getPriority());
}

///// Mesh
template<>
ComponentPeekerImpl<Mesh>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<Mesh>(actualComponent)) {
    // TODO set current state here
	currentState_.Set("mesh", "TODO");
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

///// Light
template<>
ComponentPeekerImpl<Light>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<Light>(actualComponent)) {
}

template<>
void ComponentPeekerImpl<Light>::update() {
    std::stringstream ss;
    ss << impl->color.getRgba32U();
    currentState_.Set("color", ss.str().c_str());
}

} // namespace lau
#endif
