#include "LauCommon.h"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "SkinnedMeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

#include "Light.hpp"

using namespace std;
using namespace Eigen;

namespace lau {

SkinnedMeshRenderer::SkinnedMeshRenderer() {
    shader.loadShaders("default_assets/shaders/phong_interp_animated.vs",
            "default_assets/shaders/phong_interpolated_light.fs");
}

SkinnedMeshRenderer::SkinnedMeshRenderer(const rapidjson::Value& fields) : SkinnedMeshRenderer() {
}

void SkinnedMeshRenderer::update(float dt) {
	auto mesh = gameObject->getComponent<Mesh>();
    if(wasInitialized && shader.isReady() && mesh != nullptr) {
        const auto& anims = mesh->getAnimations();
        const auto& bonePoses = mesh->getBonePoses();
        const string currAnimationName = anims.begin()->first;

        struct MatrixBlock {
            float fields[16];
        };
        vector<MatrixBlock> accumBones(bonePoses.size());
        bones.resize(bonePoses.size());

        const auto& currAnim = anims.at(currAnimationName);
        animationTime = fmod(animationTime+dt, currAnim.length);

        for(int b = 0; b < bonePoses.size(); ++b) {
            const auto bkfIt = currAnim.boneKeyframes.find(b);
            Map<Matrix4f> boneAccum(accumBones[b].fields);
            if(bkfIt != currAnim.boneKeyframes.end()) {
                Matrix4f M;
                const vector<Animation::Keyframe>& kfs = bkfIt->second;
                int kfIdx = 0;
                while(kfs[kfIdx+1].time<animationTime) {
                    assert(kfs[kfIdx+1].time>kfs[kfIdx].time);
                    kfIdx++;
                }
                const Animation::Keyframe& currKF = kfs[kfIdx]; // TODO properly handle time!
                const Animation::Keyframe& nextKF = kfs[(kfIdx+1)%kfs.size()]; // TODO properly handle time!
                // TODO create a function for building the Affine matrix!
                // Bone pose Affine matrix
                float t = (animationTime-currKF.time)/(nextKF.time-currKF.time);
                assert(t>=0.0f);
                assert(t<=1.0f);
                M.block<3,3>(0,0) = (currKF.rotation.slerp(t, nextKF.rotation)).matrix();
                M.block<3,1>(0,3) = currKF.position*(1.0f-t) + nextKF.position*t;
                float* ptr = M.data();

                // Multiply by scale. This is equivalent to performing Affine = R*S.
                Vector3f scale = currKF.scale*(1.0f-t) + nextKF.scale*t;
                ptr[0] *= scale[0]; ptr[4] *= scale[1]; ptr[8]  *= scale[2];
                ptr[1] *= scale[0]; ptr[5] *= scale[1]; ptr[9]  *= scale[2];
                ptr[2] *= scale[0]; ptr[6] *= scale[1]; ptr[10] *= scale[2];
                ptr[3] = ptr[7] = ptr[11] = 0.0;
                ptr[15] = 1.0f;

                int boneParent = mesh->getBoneParents()[b];
                assert(boneParent < b);
                if(boneParent >= 0)
                    boneAccum = Map<Matrix4f>(accumBones[boneParent].fields)*M;
                else
                    boneAccum = M;

            } else {
                boneAccum = Matrix4f::Identity();
                lerr << "No animations for bone " << b << endl;
            }

            Map<Matrix4f> boneTransform(bones[b].fields);
            boneTransform = boneAccum*bonePoses[b];
        }
    }
}

void SkinnedMeshRenderer::draw(float alpha) {
    if(!shader.isReady()) return;

	auto mesh = gameObject->getComponent<Mesh>();
    auto& transform = gameObject->transform;
    auto camera = Camera::current;
	if(mesh != nullptr) {
		if(!wasInitialized && mesh->getVBO() != nullptr) {
			mesh->getVBO()->bindAttributes(shader);
			wasInitialized = true;
		}

        shader.uniformMatrix4fv(shader.bonePosesUniformLocation, bones.size(), reinterpret_cast<const float*>(bones.data()));

		auto vbo = mesh->getVBO();
        if(vbo != nullptr) {
			vbo->bindForDrawing(shader);
            // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
            shader.uniformMatrix4fv(shader.projectionUniformLocation, 1, camera->projection.data());
            shader.uniformMatrix4fv(shader.world2cameraUniformLocation, 1, camera->world2camera.data());
            const Matrix4f& object2world = transform.getObject2WorldMatrix();
            shader.uniformMatrix4fv(shader.object2worldUniformLocation, 1, object2world.data());
            const Matrix4f& object2world_it = transform.getObject2WorldTranspOfInvMatrix();
            shader.uniformMatrix4fv(shader.object2worldITUniformLocation, 1, object2world_it.data());

            // Lights
            auto& lights = Light::allLights();
            auto lightPositions = Light::allLightPositions();
            auto lightColors = Light::allLightColors();
            shader.uniform1i(shader.numLightsUniformLocation, lights.size());
            shader.uniform3fv(shader.lightPositionsUniformLocation, lights.size(), lightPositions.data());
            shader.uniform4fv(shader.lightColorsUniformLocation, lights.size(), lightColors.data());

            glDrawElements(GL_TRIANGLES, vbo->primitivesCount(), GL_UNSIGNED_INT, 0);
        }
	}
}

/////
// Factory
#define SKINNED_MESH_RENDERER_ID 4
__LAU_CREATE_COMPONENT_INITIALIZER(lau::SkinnedMeshRenderer, SKINNED_MESH_RENDERER_ID)

} // namespace lau

