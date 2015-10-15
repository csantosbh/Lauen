#include "LauCommon.h"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

#include "Light.hpp"

#ifdef JAVASCRIPT
#include <emscripten.h>
#endif

using namespace std;
using namespace Eigen;

namespace lau {

MeshRenderer::MeshRenderer() : shaderIsReady_(false) {
    // Create shaders
    utils::IO::getInstance().requestFiles({
        "default_assets/shaders/phong_interp_animated.vs",
        "default_assets/shaders/phong_interpolated_light.fs"
    }, std::bind(&MeshRenderer::onLoadShaders, this, std::placeholders::_1));
}

MeshRenderer::MeshRenderer(const rapidjson::Value& fields) : MeshRenderer() {
}

void MeshRenderer::update(float dt) {
}

void MeshRenderer::onLoadShaders(deque<pair<bool, vector<uint8_t>>>&shaderFiles) {
    program = glCreateProgram();
    GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef GL_ES
    // GL ES
    // TODO pass the versions dynamically, since they will be chosen by the user in the interface
    const char version[] = "#version 100\n";
#else
    // GL core
    const char version[] = "#version 150\n";
#endif
    const GLchar *vsFull[] = {
        version,
        NULL,
    };
    const GLchar *fsFull[] = {
        version,
        NULL,
    };

    shaderFiles.begin()->second.push_back('\0');
    (++shaderFiles.begin())->second.push_back('\0');

    vsFull[1] = (char*)(&shaderFiles.begin()->second[0]);
    fsFull[1] = (char*)(&(++shaderFiles.begin())->second[0]);

    glShaderSource(vsId, 2, vsFull, NULL);
    glShaderSource(fsId, 2, fsFull, NULL);

    glCompileShader(vsId);
    glCompileShader(fsId);

    // Get shader compilation status
    checkShaderCompilation(vsId);
    checkShaderCompilation(fsId);

    glAttachShader(program, vsId);
    glAttachShader(program, fsId);
    glBindAttribLocation(program, vertexAttribId, "in_Position");
    glBindAttribLocation(program, normalAttribId, "in_Normal");
    glBindAttribLocation(program, skinIndexAttribId, "in_SkinIndex");
    glBindAttribLocation(program, skinWeightAttribId, "in_SkinWeight");
    glLinkProgram(program);
    glUseProgram(program);
	lout << "shader OK!" << endl;

    // Get uniform locations
    projectionUniformLocation = glGetUniformLocation(program, "projection");
    world2cameraUniformLocation = glGetUniformLocation(program, "world2camera");
    object2worldUniformLocation = glGetUniformLocation(program, "object2world");
    object2worldITUniformLocation = glGetUniformLocation(program, "object2world_it");
    numLightsUniformLocation = glGetUniformLocation(program, "numLights");
    lightPositionsUniformLocation = glGetUniformLocation(program, "lightPositions");
    lightColorsUniformLocation = glGetUniformLocation(program, "lightColors");
    bonePosesUniformLocation = glGetUniformLocation(program, "bonePoses");

#ifdef DEBUG
    if(projectionUniformLocation == -1 ||
       world2cameraUniformLocation == -1 ||
       object2worldUniformLocation == -1 ||
       object2worldITUniformLocation == -1 ||
       numLightsUniformLocation == -1 ||
       lightPositionsUniformLocation == -1 ||
       lightColorsUniformLocation == -1 ||
       bonePosesUniformLocation == -1) {
        lerr << "[error] Could not get uniform location(s)!" << endl;
        lerr << projectionUniformLocation << "; "
             << world2cameraUniformLocation << "; "
             << object2worldUniformLocation << "; "
             << object2worldITUniformLocation << "; "
             << numLightsUniformLocation << "; "
             << lightPositionsUniformLocation << "; "
             << lightColorsUniformLocation << "; "
             << bonePosesUniformLocation << endl;
    }
#endif
    shaderIsReady_ = true;
}

// TODO move this to the animation component
float animT = 0.00f;
void MeshRenderer::animUpdate(float dt) {
	auto mesh = gameObject->getComponent<Mesh>();
    if(wasInitialized && mesh != nullptr) {
        const auto& anims = mesh->getAnimations();
        const auto& bonePoses = mesh->getBonePoses();
        const string currAnimationName = anims.begin()->first;

        struct MatrixBlock {
            float fields[16];
        };
        vector<MatrixBlock> accumBones(bonePoses.size());
        vector<MatrixBlock> bones(bonePoses.size());

        const auto& currAnim = anims.at(currAnimationName);
        animT = fmod(animT+dt, currAnim.length);
        for(int b = 0; b < bonePoses.size(); ++b) {
            const auto bkfIt = currAnim.boneKeyframes.find(b);
            Map<Matrix4f> boneAccum(accumBones[b].fields);
            if(bkfIt != currAnim.boneKeyframes.end()) {
                Matrix4f M;
                const vector<Animation::Keyframe>& kfs = bkfIt->second;
                int kfIdx = 0;
                while(kfs[kfIdx+1].time<animT) {
                    assert(kfs[kfIdx+1].time>kfs[kfIdx].time);
                    kfIdx++;
                }
                const Animation::Keyframe& currKF = kfs[kfIdx]; // TODO properly handle time!
                const Animation::Keyframe& nextKF = kfs[(kfIdx+1)%kfs.size()]; // TODO properly handle time!
                // TODO create a function for building the Affine matrix!
                // Bone pose Affine matrix
                float t = (animT-currKF.time)/(nextKF.time-currKF.time);
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

        glUniformMatrix4fv(bonePosesUniformLocation, bones.size(), GL_FALSE, reinterpret_cast<const GLfloat*>(bones.data()));
    }
}

void MeshRenderer::draw(float alpha) {
    if(!shaderIsReady_) return;

	auto mesh = gameObject->getComponent<Mesh>();
    auto& transform = gameObject->transform;
    auto camera = Camera::current;
	if(mesh != nullptr) {
		if(!wasInitialized && mesh->getVBO() != nullptr) {
            GLuint attrs[] = {static_cast<GLuint>(vertexAttribId), static_cast<GLuint>(normalAttribId), static_cast<GLuint>(skinIndexAttribId), static_cast<GLuint>(skinWeightAttribId)};
			mesh->getVBO()->bindAttributes(attrs);
			wasInitialized = true;
		}

        animUpdate(0.1/300.0);

		auto vbo = mesh->getVBO();
        if(vbo != nullptr) {
            GLuint attrs[] = {static_cast<GLuint>(vertexAttribId), static_cast<GLuint>(normalAttribId), static_cast<GLuint>(skinIndexAttribId), static_cast<GLuint>(skinWeightAttribId)};
			vbo->bindForDrawing(attrs);
            // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
            glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, camera->projection.data());
            glUniformMatrix4fv(world2cameraUniformLocation, 1, GL_FALSE, camera->world2camera.data());
            const Matrix4f& object2world = transform.getObject2WorldMatrix();
            glUniformMatrix4fv(object2worldUniformLocation, 1, GL_FALSE, object2world.data());
            const Matrix4f& object2world_it = transform.getObject2WorldTranspOfInvMatrix();
            glUniformMatrix4fv(object2worldITUniformLocation, 1, GL_FALSE, object2world_it.data());

            // Lights
            auto& lights = Light::allLights();
            auto lightPositions = Light::allLightPositions();
            auto lightColors = Light::allLightColors();
            glUniform1i(numLightsUniformLocation, lights.size());
            glUniform3fv(lightPositionsUniformLocation, lights.size(), lightPositions.data());
            glUniform4fv(lightColorsUniformLocation, lights.size(), lightColors.data());

            glDrawElements(GL_TRIANGLES, vbo->primitivesCount(), GL_UNSIGNED_INT, 0);
        }
	}
}

void MeshRenderer::checkShaderCompilation(GLuint shaderId) {
    GLint compilationStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
    if(compilationStatus == GL_FALSE) {
        GLint logSize = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
        vector<char> infoLog(logSize);
        glGetShaderInfoLog(shaderId, logSize, &logSize, (char*)&infoLog[0]);
        // TODO proper error handling here
        lerr << "[Shader Error]"<<endl;
        lerr << (char*)&infoLog[0] << endl;
#if defined(DESKTOP)
        exit(1);
#elif defined(JAVASCRIPT)
        emscripten_force_exit(1);
#endif
    }
}

/////
// Factory
#define MESH_RENDERER_ID 2
__LAU_CREATE_COMPONENT_INITIALIZER(lau::MeshRenderer, MESH_RENDERER_ID)

} // namespace lau
