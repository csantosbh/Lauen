#pragma once

#include <set>

#include <rapidjson/document.h>
#include <Eigen/Eigen>

#include "Component.hpp"

namespace lau {

class Camera : public Component {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Camera(const rapidjson::Value& fields);
    ~Camera();

    void update(float dt);

    Eigen::Matrix4f projection;
    Eigen::Matrix4f world2camera;

    static const std::set<Camera*, bool(*)(const Camera*,const Camera*)>& allCameras() {
        return cameras_;
    }
    static const Camera* current;

    void draw(float temporalAlpha);

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState_;
    }
#endif

private:
#ifdef PREVIEW_MODE
    pp::VarDictionary currentState_;
    void serializeState();
#endif

    float priority;
    static bool CameraPriorityComparison(const Camera* a, const Camera* b);
    static std::set<Camera*, bool(*)(const Camera*,const Camera*)> cameras_;
};

} // namespace lau
