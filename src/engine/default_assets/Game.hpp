#pragma once

#include <vector>
#include <deque>
#include <memory>

#include "LauCommon.h"
#include "GameObject.hpp"

namespace lau {

class Game {
public:
    void init(int windowWidth, int windowHeight);

    void resize(int windowWidth, int windowHeight);

    void draw(double temporalAlpha);

    void update(double dt);

    void terminate();

private:
    std::vector<std::shared_ptr<GameObject>> gameObjects;

    ////// TODO shader&VBO&EBO classes
    //////
    GLuint bufferIds[2];
    GLuint program;
    const int vertexAttribId = 0;
    void checkShaderCompilation(GLuint shaderId);

    void createSimpleGeometry();

    void onLoadShaders(std::deque<pair<bool, std::vector<uint8_t>>>&shaderFiles);
};

} // namespace lau
