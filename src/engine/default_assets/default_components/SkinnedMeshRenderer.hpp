#pragma once

#include <vector>
#include <deque>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "DrawableComponent.hpp"
#include "opengl/ShaderProgram.hpp"

namespace lau {

class SkinnedMeshRenderer : public DrawableComponent {
public:
    SkinnedMeshRenderer();
	SkinnedMeshRenderer(const rapidjson::Value& fields);

	void update(float dt);
    void draw(float alpha);
private:
    // TODO create the post creation initialization method, like Unity, and get rid of this
    bool wasInitialized = false;
    ShaderProgram shader;
    float animationTime;
    struct Matrix4fBuffer {
        float fields[16];
    };
    std::vector<Matrix4fBuffer> bones;
};

} // namespace
