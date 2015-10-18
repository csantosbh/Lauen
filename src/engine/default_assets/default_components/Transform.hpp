#pragma once

#include <Eigen/Eigen>
#include <rapidjson/document.h>

namespace lau {

using namespace std;
class Transform {
	friend class GameObject;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;
	Transform(const rapidjson::Value& fields);

    void update(float dt);

    const Eigen::Matrix3f getRotationMatrix();
    const Eigen::Matrix4f& getObject2WorldMatrix();
    const Eigen::Matrix4f& getObject2WorldTranspOfInvMatrix();

    static void createMat4FromTransforms(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f scale, Eigen::Matrix4f& output);
    static void createInvMat4FromTransforms(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f& scale, Eigen::Matrix4f& output);
    static void createTranspOfInvMat4FromTransforms(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f scale, Eigen::Matrix4f& output);

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState_;
    }
#endif

private:
    void updateObject2World(const Eigen::Matrix4f& parent2world, const Eigen::Matrix4f parent2worldTranspOfInv);
    void updateObject2World();
    Eigen::Matrix4f object2WorldMatrix_;
    Eigen::Matrix4f object2WorldTranspOfInvMatrix_;
    bool affineMatrixUpToDate_;

#ifdef PREVIEW_MODE
    pp::VarDictionary currentState_;
    void serializeState();
#endif
};

} // namespace
