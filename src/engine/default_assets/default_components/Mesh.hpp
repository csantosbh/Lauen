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
	Mesh(const rapidjson::Value& fields);

	void update(float dt);
    VBO* getVBO();

private:
    void onLoadMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile, std::string fname);

    std::shared_ptr<VBO> vbo;
};

} // namespace
