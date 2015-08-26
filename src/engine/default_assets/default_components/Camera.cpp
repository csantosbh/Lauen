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
    nearPlane_ = static_cast<float>(fields["near"].GetDouble());
    farPlane_ = static_cast<float>(fields["far"].GetDouble());
    nearPlaneWidth_ = static_cast<float>(fields["width"].GetDouble());

    recomputeProjectionMatrix();

    world2camera <<
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 1.f;

    priority_ = static_cast<float>(fields["priority"].GetDouble());

    // Add this camera to the game's camera collection
    cameras_.insert(this);
}

Camera::~Camera() {
    // Make sure the game will forget this camera
    cameras_.erase(this);
}

void Camera::update(float dt) {
    world2camera.block<3,3>(0,0) = gameObject->transform.getRotationMatrix().transpose();
    Vector3f t = world2camera.block<3,3>(0,0)*-gameObject->transform.position;
    world2camera.block<3,1>(0,3) = t;
}

void Camera::draw(float temporalAlpha) {
    glClear(GL_COLOR_BUFFER_BIT);
    current = this;
    for(auto& gameObject: GameObject::allGameObjects()) {
        gameObject->draw(temporalAlpha);
    }
}

bool Camera::CameraPriorityComparison(const Camera* a, const Camera* b) {
    return a->priority_ < b->priority_;
}

void Camera::recomputeProjectionMatrix() {
    // TODO compute t as a function of r and the screen aspect ratio
    float t = nearPlaneWidth_;
    projection <<
        2.0f*nearPlane_/nearPlaneWidth_, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f*nearPlane_/t, 0.0f, 0.0f,
        0.0f, 0.0f, -(farPlane_+nearPlane_)/(farPlane_-nearPlane_), -2.0f*farPlane_*nearPlane_/(farPlane_-nearPlane_),
        0.0f, 0.0f, -1.0f, 0.0f;
}

void Camera::setPriority(float value) {
    priority_ = value;
    // Reposition this camera in the cameras_ set
    cameras_.erase(this);
    cameras_.insert(this);
}

float Camera::getPriority() {
    return priority_;
}

void Camera::setNearPlane(float value) {
    nearPlane_ = value;
    recomputeProjectionMatrix();
}

float Camera::getNearPlane() {
    return nearPlane_;
}

void Camera::setFarPlane(float value) {
    farPlane_ = value;
    recomputeProjectionMatrix();
}

float Camera::getFarPlane() {
    return farPlane_;
}

void Camera::setFOV(float value) {
    nearPlaneWidth_ = 2.0f*nearPlane_*tan(value/2.0f);
    recomputeProjectionMatrix();
}

float Camera::getFOV() {
    return 2.0f*atan2(nearPlaneWidth_,2.0f*nearPlane_);
}

void Camera::setNearPlaneWidth(float value) {
    nearPlaneWidth_ = value;
    recomputeProjectionMatrix();
}

float Camera::getNearPlaneWidth() {
    return nearPlaneWidth_;
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

