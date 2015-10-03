#include "../window/Screen.hpp"
#include "Factories.hpp"
#include "GameObject.hpp"
#include "Camera.hpp"
#include "LauCommon.h"

using namespace std;

using namespace rapidjson;
using namespace Eigen;

namespace lau {

std::set<Camera*, bool(*)(const Camera*,const Camera*)> Camera::cameras_ STATIC_INITIALIZER_GROUP_A (CameraPriorityComparison);
const Camera* Camera::current = nullptr;

Camera::Camera(const rapidjson::Value& fields) {
    nearPlane_ = static_cast<float>(fields["near"].GetDouble());
    farPlane_ = static_cast<float>(fields["far"].GetDouble());
    nearPlaneWidth_ = static_cast<float>(fields["width"].GetDouble());

    resetAspect(); // This will also recompute the projection matrix

    world2camera <<
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 1.f;

    priority_ = static_cast<float>(fields["priority"].GetDouble());

    // Add this camera to the game's camera collection
    cameras_.insert(this);

    // Listen to screen updates
    screenCallback_ = Screen::onWindowResize.subscribe(std::bind(&Camera::windowResizeListener, this, std::placeholders::_1, std::placeholders::_2));
}

Camera::~Camera() {
    // Make sure the game will forget this camera
    cameras_.erase(this);
    Screen::onWindowResize.unsubscribe(screenCallback_);
}

void Camera::update(float dt) {
    world2camera.block<3,3>(0,0) = gameObject->transform.getRotationMatrix().transpose();
    Vector3f t = world2camera.block<3,3>(0,0)*-gameObject->transform.position;
    world2camera.block<3,1>(0,3) = t;
}

void Camera::draw(float temporalAlpha) {
    int c[3]={rand()%100,rand()%100,rand()%100};
    glClearColor(c[0]/100.0f,c[0]/100.0f,c[0]/100.0f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const Camera* prevCamera = current;
    current = this;
    for(auto& gameObject: GameObject::allGameObjects()) {
        gameObject->draw(temporalAlpha);
    }
    // Restaure previous camera
    current = prevCamera;
}

bool Camera::CameraPriorityComparison(const Camera* a, const Camera* b) {
    return a->priority_ < b->priority_;
}

void Camera::recomputeProjectionMatrix() {
    float t = nearPlaneWidth_/aspect_;
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

void Camera::setAspect(float value) {
    customAspectProvided_ = true;
    aspect_ = value;
    recomputeProjectionMatrix();
}

// TODO listen to window changes. Only update the aspect if customAspectProvided_==false
float Camera::getAspect() {
    return aspect_;
}

void Camera::resetAspect() {
    customAspectProvided_ = false;
    aspect_ = static_cast<float>(Screen::windowWidth)/static_cast<float>(Screen::windowHeight);
    recomputeProjectionMatrix();
}

void Camera::windowResizeListener(int w, int h) {
    if(!customAspectProvided_) {
        resetAspect();
    }
}

//////
// Factory
#define CAMERA_ID 0
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Camera, CAMERA_ID)

} // namespace lau

