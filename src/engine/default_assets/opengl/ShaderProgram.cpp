#include "utils/IO.h"
#include "ShaderProgram.hpp"

#ifdef JAVASCRIPT
#include <emscripten.h>
#endif

using namespace std;

namespace lau {

ShaderProgram::ShaderProgram() : shaderIsReady_(false) {
}

void ShaderProgram::loadShaders(const char* vs, const char* fs) {
    utils::IO::getInstance().requestFiles({ vs, fs },
            std::bind(&ShaderProgram::onLoadShaders, this, std::placeholders::_1));
}

bool ShaderProgram::isReady() {
    return shaderIsReady_;
}

void ShaderProgram::uniform1i(int location, int data) {
    // TODO expose transpose parameter
    glUniform1i(location, data);
}

void ShaderProgram::uniform3fv(int location, unsigned int count, const float* buffer) {
    // TODO expose transpose parameter
    glUniform3fv(location, count, buffer);
}

void ShaderProgram::uniform4fv(int location, unsigned int count, const float* buffer) {
    // TODO expose transpose parameter
    glUniform4fv(location, count, buffer);
}

void ShaderProgram::uniformMatrix4fv(int location, unsigned int count, const float* buffer) {
    // TODO expose transpose parameter
    glUniformMatrix4fv(location, count, GL_FALSE, buffer);
}

void ShaderProgram::onLoadShaders(deque<pair<bool, vector<uint8_t>>>&shaderFiles) {
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


void ShaderProgram::checkShaderCompilation(GLuint shaderId) {
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

} // namespace
