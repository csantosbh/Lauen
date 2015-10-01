#pragma once

#include <vector>
#include <deque>
#include <memory>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "opengl/VBO.h"
#include "Component.hpp"

namespace lau {

class Mesh : public Component {
public:
    Mesh();
	Mesh(const rapidjson::Value& fields);

	void update(float dt);
    VBO* getVBO();

private:
    enum FacePrimitive { Triangle, Quad };

    void onLoadJsonMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile, std::string fname);

    std::shared_ptr<VBO> vbo;
    void computeFaceParameters(
            uint8_t facesFormat,
            int indexRemainders[8],
            Mesh::FacePrimitive& primitive,
            int& primitiveSize,
            bool& hasFaceMaterial,
            bool& hasFaceUV,
            bool& hasVertexUVs,
            bool& hasFaceNormal,
            bool& hasVertexNormals,
            bool& hasFaceColor,
            bool& hasVertexColors);
};

} // namespace
