#include <sstream>
#include "VBO.h"

using namespace std;
namespace lau {

VBO::VBO() {
	_vertexCount = 0;
}

VBO::~VBO() {
    glDeleteBuffers(2, bufferIds);
}

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<int>& indices) : dimensions(dimensions), _vertexCount(vertices.size()/dimensions) {
    // Create two VBOs: one for indices and another for vertices
    glGenBuffers(2, bufferIds);

    // Bind vertex buffer so we can start using it
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);

    // Send data to GPU
    // TODO make the GL_STATIC_DRAW an exposed, modifiable parameter. But I need to study it further first.
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lout << "[VBO] Error pushing vertex data. OpenGL error code: " << err << endl;
    }
    }
#endif

    // Bind index buffer so we can start using it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);

    // Send data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indices.size(), indices.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lout << "[VBO] Error pushing index data. OpenGL error code: " << err << endl;
    }
    }
#endif

}

// TODO make this generic as to support IPointer (int) and LPointer (double)
void VBO::bindVertexToAttribute(GLuint attributeId) {
    // Create vertex attributes so the VBO can be accessed from shaders
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
    glEnableVertexAttribArray(attributeId);
    glVertexAttribPointer(attributeId, dimensions, GL_FLOAT, GL_FALSE, 0, 0);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lout << "[VBO] Error bind vertex attribute. OpenGL error code: " << err << endl;
    }
    }
#endif
}

void VBO::bindForDrawing(GLuint attributeId) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
	glVertexAttribPointer(attributeId, dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
}

int VBO::vertexCount() {
	return this->_vertexCount;
}

} // namespace lau
