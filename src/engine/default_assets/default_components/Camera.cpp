#include "GameObject.hpp"
#include "Camera.hpp"

using namespace std;

using namespace rapidjson;
using namespace Eigen;

namespace lau {

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
}

void Camera::update(float dt) {
    world2camera.block<3,3>(0,0) = gameObject->transform.getRotationMatrix().transpose();
    Vector3f t = world2camera.block<3,3>(0,0)*-gameObject->transform.position;
    world2camera.block<3,1>(0,3) = t;
}

} // namespace lau

