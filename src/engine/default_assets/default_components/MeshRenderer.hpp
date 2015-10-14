#pragma once

#include <vector>
#include <deque>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "DrawableComponent.hpp"

namespace lau {

class MeshRenderer : public DrawableComponent {
public:
    MeshRenderer();
	MeshRenderer(const rapidjson::Value& fields);

	void update(float dt);
    void draw(float alpha);
private:
    void animUpdate(float dt);
    // TODO create the post creation initialization method, like Unity, and get rid of this
    bool wasInitialized = false;

    void onLoadShaders(std::deque<std::pair<bool, std::vector<uint8_t>>>&shaderFiles);
    void processLoadedShaders(std::deque<std::pair<bool, std::vector<uint8_t>>>&shaderFiles);

    // TODO move this to a Shader class
    GLuint program;
    const int vertexAttribId = 0;
    const int normalAttribId = 1;
    const int skinIndexAttribId = 2;
    const int skinWeightAttribId = 3;
    void checkShaderCompilation(GLuint shaderId);
    GLint projectionUniformLocation;
    GLint world2cameraUniformLocation;
    GLint object2worldUniformLocation;
    GLint object2worldITUniformLocation;
    GLint numLightsUniformLocation;
    GLint lightPositionsUniformLocation;
    GLint lightColorsUniformLocation;
    GLint bonePosesUniformLocation;
    bool shaderIsReady_;
};

} // namespace
