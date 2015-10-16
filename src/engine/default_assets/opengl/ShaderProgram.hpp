#pragma once

#include <vector>
#include <deque>
#include "LauCommon.h"

namespace lau {

class ShaderProgram {
public:
    ShaderProgram();
    void loadShaders(const char* vs, const char* fs);
    bool isReady();

    void uniform1i(int location, int value);
    void uniform3fv(int location, unsigned int count, const float* buffer);
    void uniform4fv(int location, unsigned int count, const float* buffer);
    void uniformMatrix4fv(int location, unsigned int count, const float* buffer);

    GLint projectionUniformLocation;
    GLint world2cameraUniformLocation;
    GLint object2worldUniformLocation;
    GLint object2worldITUniformLocation;
    GLint numLightsUniformLocation;
    GLint lightPositionsUniformLocation;
    GLint lightColorsUniformLocation;
    GLint bonePosesUniformLocation;

    static constexpr int vertexAttribId = 0;
    static constexpr int normalAttribId = 1;
    static constexpr int skinIndexAttribId = 2;
    static constexpr int skinWeightAttribId = 3;

private:
    void onLoadShaders(std::deque<std::pair<bool, std::vector<uint8_t>>>&shaderFiles);
    void processLoadedShaders(std::deque<std::pair<bool, std::vector<uint8_t>>>&shaderFiles);

    GLuint program;
    void checkShaderCompilation(GLuint shaderId);
    bool shaderIsReady_;
};

} // namespace
