#include "LauCommon.h"
#include "Game.hpp"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "SkinnedMeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Transform.hpp"

#include "Light.hpp"

#include "math/Matrix.hpp"
#include "math/Vector.hpp"
#include "math/Quaternion.hpp"

using namespace std;
using namespace lau::math;

namespace lau {

SkinnedMeshRenderer::SkinnedMeshRenderer() {
    shader.loadShaders("default_assets/shaders/phong_interp_animated.vs",
            "default_assets/shaders/phong_interpolated_light.fs");
}

SkinnedMeshRenderer::SkinnedMeshRenderer(const rapidjson::Value& fields) : SkinnedMeshRenderer() {
    currentAnimation = fields["animation"].GetString();
}

void SkinnedMeshRenderer::start() {
    const int DIMS = 3;
	auto mesh = gameObject->getComponent<Mesh>();
    if(mesh->isLoaded()) {
        vbo.create(DIMS, mesh->vertices, mesh->normals, mesh->faces, mesh->skinIndices, mesh->skinWeights);
        vbo.bindAttributes(shader);
    }
    else {
        mesh->onLoad.subscribe([this]() {
          Game::scheduleMainThreadTask([this]() {
            auto mesh = gameObject->getComponent<Mesh>();
            vbo.create(DIMS, mesh->vertices, mesh->normals, mesh->faces, mesh->skinIndices, mesh->skinWeights);
            vbo.bindAttributes(shader);
          });
        });
    }
}

void SkinnedMeshRenderer::update(float dt) {
	auto mesh = gameObject->getComponent<Mesh>();
    if(mesh != nullptr && mesh->isLoaded()) {
        const auto& anims = mesh->getAnimations();
        const auto& bonePoses = mesh->getBonePoses();

        if(anims.size() == 0) return;

        struct MatrixBlock {
            float fields[16];
        };
        vector<MatrixBlock> accumBones(bonePoses.size());
        bones.resize(bonePoses.size());

        const auto& currAnim = anims.at(currentAnimation);
        animationTime = fmod(animationTime+dt, currAnim.length);

        int sentinel = static_cast<int>(bonePoses.size());
        for(int b = 0; b < sentinel; ++b) {
            const vector<Animation::Keyframe>& kfs = currAnim.boneKeyframes[b];
            mat4 boneAccum = math::map<mat4>(accumBones[b].fields);
            int kfIdx = 0;

            while(kfs[kfIdx+1].time<animationTime) {
                assert(kfs[kfIdx+1].time>kfs[kfIdx].time);
                kfIdx++;
            }
            const Animation::Keyframe& currKF = kfs[kfIdx]; // TODO properly handle time!
            const Animation::Keyframe& nextKF = kfs[kfIdx+1];
            // Bone pose Affine matrix
            float t = (animationTime-currKF.time)/(nextKF.time-currKF.time);
            mat4 M;
            vec3 position = currKF.position*(1.0f-t) + nextKF.position*t;
            quaternion rotation = currKF.rotation.slerp(t, nextKF.rotation);
            vec3 scale = currKF.scale*(1.0f-t) + nextKF.scale*t;
            Transform::createMat4FromTransforms(position, rotation, scale, M);

            int boneParent = mesh->getBoneParents()[b];
            assert(boneParent < b);
            if(boneParent >= 0)
                boneAccum = math::map<mat4>(accumBones[boneParent].fields)*M;
            else
                boneAccum = M;


            mat4 boneTransform = math::map<mat4>(bones[b].fields);
            boneTransform = boneAccum*bonePoses[b];
        }
    }
}

void SkinnedMeshRenderer::draw(float alpha) {
	auto mesh = gameObject->getComponent<Mesh>();
    if(mesh == nullptr || !mesh->isLoaded())
        return;

    auto& transform = gameObject->transform;
    auto camera = Camera::current;
    shader.uniformMatrix4fv(shader.bonePosesUniformLocation, bones.size(), reinterpret_cast<const float*>(bones.data()));

    vbo.bindForDrawing(shader);
    // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
    shader.uniformMatrix4fv(shader.projectionUniformLocation, 1, camera->projection.data);
    shader.uniformMatrix4fv(shader.world2cameraUniformLocation, 1, camera->world2camera.data);
    const mat4& object2world = transform.getObject2WorldMatrix();
    shader.uniformMatrix4fv(shader.object2worldUniformLocation, 1, object2world.data);
    const mat4& object2world_it = transform.getObject2WorldTranspOfInvMatrix();
    shader.uniformMatrix4fv(shader.object2worldITUniformLocation, 1, object2world_it.data);

    // Lights
    auto& lights = Light::allLights();
    auto lightPositions = Light::allLightPositions();
    auto lightColors = Light::allLightColors();
    shader.uniform1i(shader.numLightsUniformLocation, lights.size());
    shader.uniform3fv(shader.lightPositionsUniformLocation, lights.size(), lightPositions.data());
    shader.uniform4fv(shader.lightColorsUniformLocation, lights.size(), lightColors.data());

    glDrawElements(GL_TRIANGLES, vbo.primitivesCount(), GL_UNSIGNED_INT, 0);
}

/////
// Factory
#define SKINNED_MESH_RENDERER_ID 4
__LAU_CREATE_COMPONENT_INITIALIZER(lau::SkinnedMeshRenderer, SKINNED_MESH_RENDERER_ID)

} // namespace lau

