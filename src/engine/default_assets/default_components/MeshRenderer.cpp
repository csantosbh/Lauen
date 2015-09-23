#include "LauCommon.h"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

#include "Light.hpp"

using namespace std;
using namespace Eigen;

namespace lau {

MeshRenderer::MeshRenderer() : shaderIsReady_(false) {
    // Create shaders
    utils::IO::getInstance().requestFiles({
        "default_assets/shaders/phong_interpolated_light.vs",
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
    glLinkProgram(program);
    glUseProgram(program);
	lout << "shader OK!" << endl;

    // Get uniform locations
    projectionUniformLocation = glGetUniformLocation(program, "projection");
    world2cameraUniformLocation = glGetUniformLocation(program, "world2camera");
    object2worldUniformLocation = glGetUniformLocation(program, "object2world");
    numLightsUniformLocation = glGetUniformLocation(program, "numLights");
    lightPositionsUniformLocation = glGetUniformLocation(program, "lightPositions");
    lightColorsUniformLocation = glGetUniformLocation(program, "lightColors");

#ifdef DEBUG
    if(projectionUniformLocation == -1 ||
       world2cameraUniformLocation == -1 ||
       object2worldUniformLocation == -1 ||
       numLightsUniformLocation == -1 ||
       lightPositionsUniformLocation == -1 ||
       lightColorsUniformLocation == -1) {
        lerr << "[error] Could not get uniform location(s)!" << endl;
        lerr << projectionUniformLocation << "; "
             << world2cameraUniformLocation << "; "
             << object2worldUniformLocation << "; "
             << numLightsUniformLocation << "; "
             << lightPositionsUniformLocation << "; "
             << lightColorsUniformLocation << endl;
    }
#endif
    shaderIsReady_ = true;
}

void MeshRenderer::draw(float alpha) {
    if(!shaderIsReady_) return;

	auto mesh = gameObject->getComponent<Mesh>();
    auto& transform = gameObject->transform;
    auto camera = Camera::current;
	if(mesh != nullptr) {
		if(!wasInitialized && mesh->getVBO() != nullptr) {
            GLuint attrs[] = {static_cast<GLuint>(vertexAttribId), static_cast<GLuint>(normalAttribId)};
			mesh->getVBO()->bindAttributes(attrs);
			wasInitialized = true;
		}

		auto vbo = mesh->getVBO();
        if(vbo != nullptr) {
            GLuint attrs[] = {static_cast<GLuint>(vertexAttribId), static_cast<GLuint>(normalAttribId)};
			vbo->bindForDrawing(attrs);
            // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
            Eigen::Matrix4f I = Eigen::Matrix4f::Identity();
            glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, camera->projection.data());
            glUniformMatrix4fv(world2cameraUniformLocation, 1, GL_FALSE, camera->world2camera.data());
            Matrix4f object2world = transform.parent2world*transform.getAffineTransformMatrix();
            glUniformMatrix4fv(object2worldUniformLocation, 1, GL_FALSE, object2world.data());

            // Lights
            auto& lights = Light::allLights();
            auto lightPositions = Light::allLightPositions();
            auto lightColors = Light::allLightColors();
            glUniform1i(numLightsUniformLocation, lights.size());
            glUniform3fv(lightPositionsUniformLocation, lights.size(), lightPositions.data());
            glUniform4fv(lightColorsUniformLocation, lights.size(), lightColors.data());

            glDrawElements(GL_TRIANGLE_STRIP, vbo->vertexCount(), GL_UNSIGNED_INT, 0);
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
        lerr << (char*)&infoLog[0] << endl;
#ifndef NACL
        exit(0);
#endif
    }
}

/////
// Factory
#define MESH_RENDERER_ID 2
__LAU_CREATE_COMPONENT_INITIALIZER(lau::MeshRenderer, MESH_RENDERER_ID)

} // namespace lau
