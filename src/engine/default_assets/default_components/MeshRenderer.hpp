#pragma once

#include <vector>
#include <deque>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "opengl/VBO.h"
#include "DrawableComponent.hpp"

namespace lau {

class MeshRenderer : public DrawableComponent {
public:
	MeshRenderer(const rapidjson::Value& fields);

	void update(float dt);
    void draw(float alpha);
private:
    // TODO create the post creation initialization method, like Unity, and get rid of this
    bool wasInitialized = false;

    void onLoadShaders(std::deque<std::pair<bool, std::vector<uint8_t>>>&shaderFiles);

    // TODO move this to a Shader class
    GLuint program;
    const int vertexAttribId = 0;
    void checkShaderCompilation(GLuint shaderId);
};

} // namespace