#include "Factories.hpp"
#include "Transform.hpp"
#include "LauCommon.h"

using namespace lau::math;

namespace lau {

Transform::Transform(const rapidjson::Value& serializedTransform) :
    rotation(quaternion::identity()),
    object2WorldMatrix_({0,0,0,0,
                         0,0,0,0,
                         0,0,0,0,
                         0,0,0,1}),
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

    rotation = quaternion(eulerAngles[2], {0,0,1}) *
               quaternion(eulerAngles[1], {0,1,0}) *
               quaternion(eulerAngles[0], {1,0,0});

#ifdef PREVIEW_MODE
    serializeState();
#endif
}

mat3 Transform::getRotationMatrix() const {
    return this->rotation.matrix();
}

void Transform::update(float) {
#ifdef PREVIEW_MODE
    serializeState();
#endif
}

const mat4& Transform::getObject2WorldMatrix() {
    return object2WorldMatrix_;
}

const mat4& Transform::getObject2WorldTranspOfInvMatrix() {
    return object2WorldTranspOfInvMatrix_;
}

void Transform::updateObject2World(const mat4& parent2world, const mat4& parent2worldTranspOfInv) {
	mat4 object2parent;
    createMat4FromTransforms(position, rotation, scale, object2parent);

	// Now transform from obj2parent to obj2world
	object2WorldMatrix_ = parent2world * object2parent;

	////
    // Update the object2world^-1^t matrix
	mat4 obj2parentTranspOfInvMatrix_;
    createTranspOfInvMat4FromTransforms(position, rotation, scale, obj2parentTranspOfInvMatrix_);

	// Now apply the parent transform
	object2WorldTranspOfInvMatrix_ = parent2worldTranspOfInv * obj2parentTranspOfInvMatrix_;
}

void Transform::updateObject2World() {
    createMat4FromTransforms(position, rotation, scale, object2WorldMatrix_);

	////
    // Update the object2world^-1^t matrix
    createTranspOfInvMat4FromTransforms(position, rotation, scale, object2WorldTranspOfInvMatrix_);
}

void Transform::createMat4FromTransforms(const vec3& position, const quaternion& rotation, const vec3& scale, mat4& output) {
    output.copyBlock(rotation.matrix(), 0, 0);
    output.copyBlock(position, 0, 3);
    float* ptr = output.data;

    // Multiply by scale. This is equivalent to performing Affine = R*S.
    ptr[0] *= scale[0]; ptr[4] *= scale[1]; ptr[8]  *= scale[2];
    ptr[1] *= scale[0]; ptr[5] *= scale[1]; ptr[9]  *= scale[2];
    ptr[2] *= scale[0]; ptr[6] *= scale[1]; ptr[10] *= scale[2];
    ptr[3] = ptr[7] = ptr[11] = 0.0;
    ptr[15] = 1.0f;
}

void Transform::createInvMat4FromTransforms(const vec3& position, const quaternion& rotation, const vec3& scale, mat4& output) {
    const mat3 rotMat = rotation.matrix().transpose();
    output.copyBlock(rotMat, 0, 0);
    output.copyBlock(rotMat * -position, 0, 3);

    float* ptr = output.data;

    // Multiply by scale^-1. This is equivalent to performing Affine = S^-1*R^-1.
    const float inv_scale[] = {1.0f/scale[0], 1.0f/scale[1], 1.0f/scale[2]};
    ptr[0] *= inv_scale[0]; ptr[4] *= inv_scale[0]; ptr[8]  *= inv_scale[0]; ptr[12] *= inv_scale[0];
    ptr[1] *= inv_scale[1]; ptr[5] *= inv_scale[1]; ptr[9]  *= inv_scale[1]; ptr[13] *= inv_scale[1];
    ptr[2] *= inv_scale[2]; ptr[6] *= inv_scale[2]; ptr[10] *= inv_scale[2]; ptr[14] *= inv_scale[2];
    ptr[3] = ptr[7] = ptr[11] = 0.0;
    ptr[15] = 1.0f;

}

void Transform::createTranspOfInvMat4FromTransforms(const vec3& position, const quaternion& rotation, const vec3 scale, mat4& output) {
    createInvMat4FromTransforms(position, rotation, scale, output);
    output.transposeInPlace();
}

#ifdef PREVIEW_MODE
void Transform::serializeState() {
	// TODO figure out how the eulerangles are being returned (in which order?), and make sure it is consistent with the order in the Editor
	vec3 eulerAngles = this->rotation.eulerAngles(0, 1, 2);

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
