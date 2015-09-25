#include "Factories.hpp"
#include "Transform.hpp"
#include "LauCommon.h"

using namespace Eigen;

namespace lau {

Transform::Transform(const rapidjson::Value& serializedTransform) :
    rotation(Quaternionf::Identity()),
    affineMatrixUpToDate_(false) {
    const rapidjson::Value& fields = serializedTransform["fields"];
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

    object2WorldMatrix_ << 0,0,0,0,
                              0,0,0,0,
                              0,0,0,0,
                              0,0,0,1;

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

const Matrix3f Transform::getRotationMatrix() {
    return this->rotation.toRotationMatrix();
}

void Transform::update(float dt) {
#ifdef PREVIEW_MODE
    serializeState();
#endif
}

const Matrix4f& Transform::getObject2WorldMatrix() {
    return object2WorldMatrix_;
}

const Matrix4f& Transform::getObject2WorldTranspOfInvMatrix() {
    return object2WorldTranspOfInvMatrix_;
}

void Transform::updateObject2World(const Matrix4f& parent2world, const Eigen::Matrix4f parent2worldTranspOfInv) {
	Matrix4f object2parent;
	object2parent.data()[15] = 1.0f;
    object2parent.block<3,3>(0,0) = rotation.matrix();
    object2parent.block<3,1>(0,3) = position;
    float* ptr = object2parent.data();

    // Multiply by scale. This is equivalent to performing Affine = R*S.
    ptr[0] *= scale[0]; ptr[1] *= scale[1]; ptr[2] *= scale[2];
    ptr[4] *= scale[0]; ptr[5] *= scale[1]; ptr[6] *= scale[2];
    ptr[8] *= scale[0]; ptr[9] *= scale[1]; ptr[10] *= scale[2];

	// Now transform from obj2parent to obj2world
	object2WorldMatrix_ = parent2world * object2parent;

	////
    // Update the object2world^-1^t matrix
	Matrix4f obj2parentTranspOfInvMatrix_;
	obj2parentTranspOfInvMatrix_.data()[15] = 1.0f;
    obj2parentTranspOfInvMatrix_.block<3,3>(0,0) = rotation.matrix().transpose();
    obj2parentTranspOfInvMatrix_.block<3,1>(0,3) = obj2parentTranspOfInvMatrix_.block<3,3>(0,0) * -position;
    ptr = obj2parentTranspOfInvMatrix_.data();

    // Multiply by scale. This is equivalent to performing Affine = R*S.
    float inv_scale[] = {1.0f/scale[0], 1.0f/scale[1], 1.0f/scale[2]};
    ptr[0] *= inv_scale[0]; ptr[1] *= inv_scale[0]; ptr[2] *= inv_scale[0];
    ptr[4] *= inv_scale[1]; ptr[5] *= inv_scale[1]; ptr[6] *= inv_scale[1];
    ptr[8] *= inv_scale[2]; ptr[9] *= inv_scale[2]; ptr[10] *= inv_scale[2];

    // Transpose it
    obj2parentTranspOfInvMatrix_.transposeInPlace();

	// Now apply the parent transform
	object2WorldTranspOfInvMatrix_ = parent2worldTranspOfInv * obj2parentTranspOfInvMatrix_;
}

// TODO figure out a way to reuse code from updateObject2World above
void Transform::updateObject2World() {
    object2WorldMatrix_.block<3,3>(0,0) = rotation.matrix();
    object2WorldMatrix_.block<3,1>(0,3) = position;
    float* ptr = object2WorldMatrix_.data();

    // Multiply by scale. This is equivalent to performing Affine = R*S.
    ptr[0] *= scale[0]; ptr[1] *= scale[1]; ptr[2] *= scale[2];
    ptr[4] *= scale[0]; ptr[5] *= scale[1]; ptr[6] *= scale[2];
    ptr[8] *= scale[0]; ptr[9] *= scale[1]; ptr[10] *= scale[2];

	////
    // Update the object2world^-1^t matrix
    object2WorldTranspOfInvMatrix_.block<3,3>(0,0) = rotation.matrix().transpose();
    object2WorldTranspOfInvMatrix_.block<3,1>(0,3) = object2WorldTranspOfInvMatrix_.block<3,3>(0,0) * -position;
    ptr = object2WorldTranspOfInvMatrix_.data();

    // Multiply by scale. This is equivalent to performing Affine = R*S.
    float inv_scale[] = {1.0f/scale[0], 1.0f/scale[1], 1.0f/scale[2]};
    ptr[0] *= inv_scale[0]; ptr[1] *= inv_scale[0]; ptr[2] *= inv_scale[0];
    ptr[4] *= inv_scale[1]; ptr[5] *= inv_scale[1]; ptr[6] *= inv_scale[1];
    ptr[8] *= inv_scale[2]; ptr[9] *= inv_scale[2]; ptr[10] *= inv_scale[2];

    // Transpose it
    object2WorldTranspOfInvMatrix_.transposeInPlace();
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

    pp::VarDictionary fields;
	fields.Set("position", pos);
	fields.Set("scale", scale);
	fields.Set("rotation", rot);
    currentState_.Set("fields", fields);
}
#endif

} // namespace lau
