#pragma once

#include <set>

#include <rapidjson/document.h>
#include <Eigen/Eigen>

#include "Component.hpp"
#include "../utils/Callback.hpp"

#include "math/Matrix.hpp"

namespace lau {

class Camera : public Component {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Camera(const rapidjson::Value& fields);
    ~Camera();

    void update(float dt);

    math::mat4 projection;
    math::mat4 world2camera;

    static const std::set<Camera*, bool(*)(const Camera*,const Camera*)>& allCameras() {
        return cameras_;
    }
    static const Camera* current;

    void draw(float temporalAlpha);

    void setPriority(float value);
    float getPriority();
    void setNearPlane(float value);
    float getNearPlane();
    void setFarPlane(float value);
    float getFarPlane();
    // In Radians
    void setFOV(float value);
    // In Radians
    float getFOV();
    void setNearPlaneWidth(float value);
    float getNearPlaneWidth();

    // Aspect: width/height
    void setAspect(float value);
    float getAspect();
    void resetAspect();

private:
    static bool CameraPriorityComparison(const Camera* a, const Camera* b);

    static std::set<Camera*, bool(*)(const Camera*,const Camera*)> cameras_;

    void recomputeProjectionMatrix();
    void windowResizeListener(int w, int h);

    float priority_;
    float farPlane_;
    float nearPlane_;
    float nearPlaneWidth_;

    float aspect_;
    bool customAspectProvided_;
    Callback<int,int>::CallbackID screenCallback_;

    float totalT_;
};

} // namespace lau
