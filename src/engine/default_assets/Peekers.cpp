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

///// Transform
template<>
ComponentPeekerImpl<Transform>::ComponentPeekerImpl(std::shared_ptr<Component> actualComponent) : impl(std::dynamic_pointer_cast<Transform>(actualComponent)) {
}

template<>
void ComponentPeekerImpl<Transform>::update() {
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
}

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
