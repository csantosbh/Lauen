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
    ShaderProgram shader;
    VBO vbo;
};

} // namespace
