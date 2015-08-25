#pragma once

#include <Eigen/Eigen>
#include <rapidjson/document.h>

namespace lau {

using namespace std;
class Transform {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Eigen::Vector3f position;
	Eigen::Vector3f scale;
	Eigen::Quaternionf rotation;
	Transform(const rapidjson::Value& fields);

    void update(float dt);

    const Eigen::Matrix3f getRotationMatrix();
    const Eigen::Matrix4f& getAffineTransformMatrix();

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState_;
    }
#endif

private:
    Eigen::Matrix4f affineTransformMatrix_;
    bool affineMatrixUpToDate_;

#ifdef PREVIEW_MODE
    pp::VarDictionary currentState_;
    void serializeState();
#endif
};

} // namespace
