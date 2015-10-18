#pragma once

#include <vector>
#include <deque>
#include <string>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "DrawableComponent.hpp"
#include "opengl/ShaderProgram.hpp"
#include "opengl/VBO.hpp"

namespace lau {

class SkinnedMeshRenderer : public DrawableComponent {
public:
    SkinnedMeshRenderer();
	SkinnedMeshRenderer(const rapidjson::Value& fields);

    void start();
	void update(float dt);
    void draw(float alpha);
    std::string currentAnimation;
private:
    float animationTime;
    struct Matrix4fBuffer {
        float fields[16];
    };
    std::vector<Matrix4fBuffer> bones;

    ShaderProgram shader;
    VBO vbo;
};

} // namespace
