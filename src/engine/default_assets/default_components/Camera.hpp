#pragma once

#include <rapidjson/document.h>
#include <Eigen/Eigen>

#include "Component.hpp"

namespace lau {

class Camera : public Component {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Camera(const rapidjson::Value& fields);

    void update(float dt);

    Eigen::Matrix4f projection;
    Eigen::Matrix4f world2camera;
};

} // namespace lau
