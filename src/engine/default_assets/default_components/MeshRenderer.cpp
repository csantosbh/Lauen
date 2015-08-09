#include "Factories.hpp"
#include "GameObject.hpp"
#include "Mesh.hpp"
#include "MeshRenderer.hpp"
#include "utils/IO.h"
#include "LauCommon.h"

using namespace std;

namespace lau {

MeshRenderer::MeshRenderer(const rapidjson::Value& fields) {
    // Create shaders
    utils::IO::getInstance().requestFiles({
        "default_assets/shaders/basic.vs",
        "default_assets/shaders/basic.fs"
    }, std::bind(&MeshRenderer::onLoadShaders, this, std::placeholders::_1));
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

    lerr << "vs:"<<endl;
    lerr << vsFull[1] << endl;
    lerr << "fs:"<<endl;
    lerr << fsFull[1] << endl;

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
    glLinkProgram(program);
    glUseProgram(program);
	lout << "shader OK!" << endl;
}

void MeshRenderer::draw(float alpha) {
	auto mesh = gameObject->getComponent<Mesh>();
	if(mesh != nullptr) {
		if(!wasInitialized) {
			mesh->getVBO().bindVertexToAttribute(vertexAttribId);
			wasInitialized = true;
		}

		auto& vbo = mesh->getVBO();
		vbo.bindForDrawing(vertexAttribId);
		glDrawElements(GL_TRIANGLE_STRIP, vbo.vertexCount(), GL_UNSIGNED_INT, 0);
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
template<>
int Component::getComponentId<lau::MeshRenderer>() {
	return MESH_RENDERER_ID;
}

template<>
shared_ptr<Component> Factories::componentInternalFactory<lau::MeshRenderer>(shared_ptr<GameObject>& gameObj, const rapidjson::Value& fields) {
	lau::MeshRenderer* ptr = new lau::MeshRenderer(fields);

	shared_ptr<Component> result;
#ifndef PREVIEW_MODE
	result = shared_ptr<Component>(dynamic_cast<Component*>(ptr));
#else
	result = shared_ptr<Component>(dynamic_cast<Component*>(new ComponentPeeker<lau::MeshRenderer>(shared_ptr<lau::MeshRenderer>(ptr))));
#endif

	result->setId(MESH_RENDERER_ID);

	return result;
}

template<>
struct Initializer<lau::MeshRenderer> {
	Initializer() {
		Factories::componentInstanceFactories[MESH_RENDERER_ID] = &Factories::componentInternalFactory<lau::MeshRenderer>;
	}
	static Initializer<lau::MeshRenderer> instance;
};
Initializer<lau::MeshRenderer> Initializer<lau::MeshRenderer>::instance;

} // namespace lau
