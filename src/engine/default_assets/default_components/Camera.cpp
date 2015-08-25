#include "Factories.hpp"
#include "GameObject.hpp"
#include "Camera.hpp"

using namespace std;

using namespace rapidjson;
using namespace Eigen;

namespace lau {

std::set<Camera*, bool(*)(const Camera*,const Camera*)> Camera::cameras_ STATIC_INITIALIZER_GROUP_A (CameraPriorityComparison);
const Camera* Camera::current;

Camera::Camera(const rapidjson::Value& fields) {
    float n = static_cast<float>(fields["near"].GetDouble());
    float f = static_cast<float>(fields["far"].GetDouble());
    float r = static_cast<float>(fields["width"].GetDouble())/2.0f;
    // TODO compute t as a function of r and the screen aspect ratio
    float t = r;
    projection <<
        n/r, 0.0f, 0.0f, 0.0f,
        0.0f, n/t, 0.0f, 0.0f,
        0.0f, 0.0f, -(f+n)/(f-n), -2.0f*f*n/(f-n),
        0.0f, 0.0f, -1.0f, 0.0f;

    world2camera <<
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 1.f;

    priority = static_cast<float>(fields["priority"].GetDouble());

    // Add this camera to the game's camera collection
    cameras_.insert(this);

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

Camera::~Camera() {
    // Make sure the game will forget this camera
    cameras_.erase(this);
}

#ifdef PREVIEW_MODE
void Camera::serializeState() {
	// TODO properly broadcast all these values

	currentState_.Set("near", 0);
	currentState_.Set("far", 0);
	currentState_.Set("width", 0);
	currentState_.Set("priority", 0);
}
#endif

void Camera::update(float dt) {
    world2camera.block<3,3>(0,0) = gameObject->transform.getRotationMatrix().transpose();
    Vector3f t = world2camera.block<3,3>(0,0)*-gameObject->transform.position;
    world2camera.block<3,1>(0,3) = t;

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

void Camera::draw(float temporalAlpha) {
    glClear(GL_COLOR_BUFFER_BIT);
    current = this;
    for(auto& gameObject: GameObject::allGameObjects()) {
        gameObject->draw(temporalAlpha);
    }
}

bool Camera::CameraPriorityComparison(const Camera* a, const Camera* b) {
    return a->priority < b->priority;
}

//////
// Factory
#define CAMERA_ID 0
template<>
int Component::getComponentId<lau::Camera>() {
	return CAMERA_ID;
}

template<>
shared_ptr<Component> Factories::componentInternalFactory<lau::Camera>(shared_ptr<GameObject>& gameObj, const rapidjson::Value& fields) {
	lau::Camera* ptr = new lau::Camera(fields);

	shared_ptr<Component> result;
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#ifdef PREVIEW_MODE
	result->lau_peeker__ = shared_ptr<ComponentPeeker>(dynamic_cast<ComponentPeeker*>(new ComponentPeekerImpl<lau::Camera>(result)));
#endif

	result->setId(CAMERA_ID);

	return result;
}

// TODO posso fazer esse initializer todo com templates, e soh instanciar o template em cada classe. O mesmo provavelmente para componentInternalFactory.
template<>
struct Initializer<lau::Camera> {
	Initializer() {
		Factories::componentInstanceFactories[CAMERA_ID] = &Factories::componentInternalFactory<lau::Camera>;
	}
	static Initializer<lau::Camera> instance;
};
Initializer<lau::Camera> Initializer<lau::Camera>::instance STATIC_INITIALIZER_GROUP_C;

} // namespace lau

