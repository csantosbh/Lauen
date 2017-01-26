#pragma once

#include <Eigen/Eigen>
#include <rapidjson/document.h>
#include "math/Vector.hpp"
#include "math/Matrix.hpp"
#include "math/Quaternion.hpp"

namespace lau {

using namespace std;
class Transform {
    friend class GameObject;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    math::vec3 position;
    math::quaternion rotation;
    math::vec3 scale;
    Transform(const rapidjson::Value& fields);

    void update(float dt);

    math::mat3 getRotationMatrix() const;
    const math::mat4& getObject2WorldMatrix();
    const math::mat4& getObject2WorldTranspOfInvMatrix();

    static void createMat4FromTransforms(const math::vec3& position, const math::quaternion& rotation, const math::vec3& scale, math::mat4& output);
    static void createInvMat4FromTransforms(const math::vec3& position, const math::quaternion& rotation, const math::vec3& scale, math::mat4& output);
    static void createTranspOfInvMat4FromTransforms(const math::vec3& position, const math::quaternion& rotation, const math::vec3 scale, math::mat4& output);

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState_;
    }
#endif

private:
    void updateObject2World(const math::mat4& parent2world, const math::mat4& parent2worldTranspOfInv);
    void updateObject2World();
    math::mat4 object2WorldMatrix_;
    math::mat4 object2WorldTranspOfInvMatrix_;
    bool affineMatrixUpToDate_;

#ifdef PREVIEW_MODE
    pp::VarDictionary currentState_;
    void serializeState();
#endif
};

} // namespace
