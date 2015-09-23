#include <sstream>
#include "VBO.h"

using namespace std;
namespace lau {

VBO::VBO() {
	vertexCount_ = 0;
}

VBO::~VBO() {
    glDeleteBuffers(2, bufferIds_);
}

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<int>& indices) : dimensions_(dimensions), vertexCount_(vertices.size()/dimensions) {
    // Create two VBOs: one for indices and another for vertices
    glGenBuffers(2, bufferIds_);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error generating VBO buffer. OpenGL error code: " << err << endl;
    }
    }
#endif

    // Bind vertex buffer so we can start using it
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);

    // Send data to GPU
    // TODO make the GL_STATIC_DRAW an exposed, modifiable parameter. But I need to study it further first.
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error pushing vertex data. OpenGL error code: " << err << "; buffer id: " << bufferIds_[0] << endl;
    }
    }
#endif

    // Bind index buffer so we can start using it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);

    // Send data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indices.size(), indices.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error pushing index data. OpenGL error code: " << err << endl;
    }
    }
#endif

    bufferFormat_ = VBOFormat::Vf;
    stride_ = sizeof(float) * dimensions_;
}

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<float>& normals, vector<int>& indices) : dimensions_(dimensions), vertexCount_(vertices.size()/dimensions) {
    // Create two VBOs: one for indices and another for vertices
    glGenBuffers(2, bufferIds_);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error generating VBO buffer. OpenGL error code: " << err << endl;
    }
    }
#endif

    vector<float> verticesAndNormals;
    verticesAndNormals.reserve(dimensions_ * vertexCount_ * 2);
    for(int i = 0; i < vertexCount_; ++i) {
        int baseIdx = i*dimensions_;
        // Vertex
        for(int d = 0; d < dimensions_; ++d)
            verticesAndNormals.push_back(vertices[baseIdx + d]);
        // Normal
        for(int d = 0; d < dimensions_; ++d)
            verticesAndNormals.push_back(normals[baseIdx + d]);
    }

    // Bind vertex buffer so we can start using it
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);

    // Send data to GPU
    // TODO make the GL_STATIC_DRAW an exposed, modifiable parameter. But I need to study it further first.
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verticesAndNormals.size(), verticesAndNormals.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error pushing vertex data. OpenGL error code: " << err << "; buffer id: " << bufferIds_[0] << endl;
    }
    }
#endif

    // Bind index buffer so we can start using it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);

    // Send data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indices.size(), indices.data(), GL_STATIC_DRAW);

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error pushing index data. OpenGL error code: " << err << endl;
    }
    }
#endif

    bufferFormat_ = VBOFormat::VNf;
    const int numComponents_ = 2;
    stride_ = sizeof(float) * dimensions_ * numComponents_;
}

// TODO make this generic as to support IPointer (int) and LPointer (double)
void VBO::bindAttributes(const GLuint* attributeIds) {
    // Create vertex attributes so the VBO can be accessed from shaders
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);

    if((bufferFormat_&VBOFormat::Vf) != 0) {
        glEnableVertexAttribArray(attributeIds[0]);
        glVertexAttribPointer(attributeIds[0], dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    }
    if((bufferFormat_&VBOFormat::VNf) != 0) {
        glEnableVertexAttribArray(attributeIds[1]);
        glVertexAttribPointer(attributeIds[1], dimensions_, GL_FLOAT, GL_FALSE, stride_, reinterpret_cast<GLvoid*>(sizeof(float)*dimensions_));
    }

#ifdef DEBUG
    {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        lerr << "[VBO] Error bind vertex attribute. OpenGL error code: " << err << endl;
    }
    }
#endif
}

void VBO::bindForDrawing(const GLuint* attributeIds) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);

    if((bufferFormat_&VBOFormat::Vf) != 0)
        glVertexAttribPointer(attributeIds[0], dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    if((bufferFormat_&VBOFormat::VNf) != 0)
        glVertexAttribPointer(attributeIds[1], dimensions_, GL_FLOAT, GL_FALSE, stride_, reinterpret_cast<GLvoid*>(sizeof(float)*dimensions_));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);
}

int VBO::vertexCount() {
	return this->vertexCount_;
}

} // namespace lau
