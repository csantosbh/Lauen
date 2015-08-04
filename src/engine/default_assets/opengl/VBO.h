#pragma once
#include <vector>

#include "LauCommon.h"

namespace lau {

// TODO create an EBO class
class VBO {
public:
    VBO();
    VBO(uint8_t dimensions, std::vector<float>& vertices, std::vector<int>& indices);
	~VBO();

    void bindVertexToAttribute(GLuint attributeId);
	void bindForDrawing(GLuint attributeId);
	int vertexCount();

private:
    uint8_t dimensions;
	int _vertexCount;
    GLuint bufferIds[2];
};

} // namespace lau
