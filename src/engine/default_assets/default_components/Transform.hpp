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
	Transform(const rapidjson::Value& fields);

    void update(float dt);

    const Eigen::Matrix3f& getRotationMatrix();

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState_;
    }
#endif

private:
	Eigen::Quaternionf rotation_;
    Eigen::Matrix3f rotationMatrix_;
    bool rotationMatrixUpToDate_;

#ifdef PREVIEW_MODE
    pp::VarDictionary currentState_;
    void serializeState();
#endif
};

} // namespace
