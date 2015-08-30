#include "Factories.hpp"
#include "Transform.hpp"
#include "LauCommon.h"

using namespace Eigen;

namespace lau {

Transform::Transform(const rapidjson::Value& fields) :
    rotation(Quaternionf::Identity()),
    parent2world(Matrix4f::Identity()),
    affineMatrixUpToDate_(false) {
    const auto& pos = fields["position"];
    const auto& rot = fields["rotation"];
    const auto& scale = fields["scale"];
    float eulerAngles[3];

    for(int i = 0; i < 3; ++i) {
        this->position[i] = pos[i].GetDouble();
        eulerAngles[i] = rot[i].GetDouble();
        this->scale[i] = scale[i].GetDouble();
    }

    rotation = Quaternionf(
            AngleAxisf(eulerAngles[0], Vector3f::UnitX()) *
            AngleAxisf(eulerAngles[1], Vector3f::UnitY()) *
            AngleAxisf(eulerAngles[2], Vector3f::UnitZ())
            );

    affineTransformMatrix_ << 0,0,0,0,
                              0,0,0,0,
                              0,0,0,0,
                              0,0,0,1;

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

const Matrix3f Transform::getRotationMatrix() {
    return getAffineTransformMatrix().block<3,3>(0,0);
}

void Transform::update(float dt) {
    //rotation = rotation*AngleAxisf(0.1 * M_PI/180.0f, Vector3f::UnitZ());

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

const Matrix4f& Transform::getAffineTransformMatrix() {
    if(!affineMatrixUpToDate_) {
        affineTransformMatrix_.block<3,3>(0,0) = rotation.matrix();
        affineTransformMatrix_.block<3,1>(0,3) = position;
    }
    return affineTransformMatrix_;
}

#ifdef PREVIEW_MODE
void Transform::serializeState() {
	// TODO figure out how the eulerangles are being returned (in which order?), and make sure it is consistent with the order in the Editor
	Vector3f eulerAngles = this->rotation.toRotationMatrix().eulerAngles(0, 1, 2);

	pp::VarArray pos;
	pp::VarArray rot;
	pp::VarArray scale;
	for(int i = 0; i < 3; ++i) {
		pos.Set(i, this->position[i]);
		rot.Set(i, eulerAngles[i]);
		scale.Set(i, this->scale[i]);
	}

	currentState_.Set("position", pos);
	currentState_.Set("scale", scale);
	currentState_.Set("rotation", rot);
}
#endif

} // namespace lau
