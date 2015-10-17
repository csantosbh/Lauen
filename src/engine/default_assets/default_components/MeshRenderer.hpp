#pragma once

#include <vector>
#include <deque>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "DrawableComponent.hpp"
#include "opengl/VBO.hpp"
#include "opengl/ShaderProgram.hpp"

namespace lau {

class MeshRenderer : public DrawableComponent {
public:
    MeshRenderer();
	MeshRenderer(const rapidjson::Value& fields);

    void start();
	void update(float dt);
    void draw(float alpha);
private:
    // TODO create the post creation initialization method, like Unity, and get rid of this
    ShaderProgram shader;
    VBO vbo;
};

} // namespace
