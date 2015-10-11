#include <cassert>
#include <stdio.h>
#include <sstream>
#include "VBO.h"

using namespace std;
namespace lau {

VBO::VBO() {
	primitivesCount_ = 0;
}

VBO::~VBO() {
    glDeleteBuffers(2, bufferIds_);
}

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<int>& indices) : dimensions_(dimensions), primitivesCount_(indices.size()) {
    int vertexCount = vertices.size()/dimensions;
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

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<float>& normals, vector<int>& indices) : dimensions_(dimensions), primitivesCount_(indices.size()) {
    assert(vertices.size() == normals.size());
    int vertexCount = vertices.size()/dimensions;

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
    verticesAndNormals.reserve(vertices.size()+normals.size());
    for(int i = 0; i < vertices.size(); i += dimensions_) {
        // Vertex
        for(int d = 0; d < dimensions_; ++d) {
            verticesAndNormals.push_back(vertices[i + d]);
        }
        // Normal
        for(int d = 0; d < dimensions_; ++d)
            verticesAndNormals.push_back(normals[i + d]);
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

VBO::VBO(uint8_t dimensions, vector<float>& vertices, vector<float>& normals, vector<int>& indices, vector<int>& skinIndices, vector<float>& skinWeights) : dimensions_(dimensions), primitivesCount_(indices.size()) {
    assert(vertices.size() == normals.size());
    int vertexCount = vertices.size()/dimensions;

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

    const int BONES_PER_VERTEX = 2; // TODO make the number of vertices a globally available constant (it is also defined on Mesh.cpp). This must also reflect on the shader.
    vector<uint8_t> modelBuffer;
    modelBuffer.reserve(vertices.size()*sizeof(float)+normals.size()*sizeof(float)+BONES_PER_VERTEX*sizeof(int)+BONES_PER_VERTEX*sizeof(int));
    for(int i = 0; i < vertices.size(); i += dimensions_) {
        // Vertex
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(vertices.data()+i),
                reinterpret_cast<uint8_t*>(vertices.data()+i+dimensions_));
        // Normal
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(normals.data()+i),
                reinterpret_cast<uint8_t*>(normals.data()+i+dimensions_));
        // Skin indices
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(skinIndices.data()+i),
                reinterpret_cast<uint8_t*>(skinIndices.data()+i+dimensions_));
        // Skin weights
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(skinWeights.data()+i),
                reinterpret_cast<uint8_t*>(skinWeights.data()+i+dimensions_));
    }

    // Bind vertex buffer so we can start using it
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);

    // Send data to GPU
    // TODO make the GL_STATIC_DRAW an exposed, modifiable parameter. But I need to study it further first.
    glBufferData(GL_ARRAY_BUFFER, modelBuffer.size(), modelBuffer.data(), GL_STATIC_DRAW);

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

    bufferFormat_ = VBOFormat::VNfSkinIW;
    stride_ = modelBuffer.size()/vertices.size()*dimensions_;
}

// TODO make this generic as to support IPointer (int) and LPointer (double)
// TODO the parameter for this function should be a struct, with named fields instead of an array
void VBO::bindAttributes(const GLuint* attributeIds) {
    // Create vertex attributes so the VBO can be accessed from shaders
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);

    // TODO usar switch
    if((bufferFormat_&0x1) != 0) {
        // Vertex
        glEnableVertexAttribArray(attributeIds[0]);
        glVertexAttribPointer(attributeIds[0], dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    }
    if((bufferFormat_&0x2) != 0) {
        // Normal
        glEnableVertexAttribArray(attributeIds[1]);
        glVertexAttribPointer(attributeIds[1], dimensions_, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_));
    }
    if((bufferFormat_&0x4) != 0) {
        // Skin index&weight
        const int BONES_PER_VERTEX = 2;
        glEnableVertexAttribArray(attributeIds[2]);
        glEnableVertexAttribArray(attributeIds[3]);
#ifndef GL_ES
        glVertexAttribIPointer(attributeIds[2], BONES_PER_VERTEX, GL_INT, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#else
        glVertexAttribPointer(attributeIds[2], BONES_PER_VERTEX, GL_INT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#endif
        glVertexAttribPointer(attributeIds[3], BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2 + sizeof(int)*BONES_PER_VERTEX));
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

    if((bufferFormat_&0x1) != 0) {
        glVertexAttribPointer(attributeIds[0], dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    }
    if((bufferFormat_&0x2) != 0) {
        glVertexAttribPointer(attributeIds[1], dimensions_, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_));
    }
    if((bufferFormat_&0x4) != 0) {
        // Skin index&weight
        const int BONES_PER_VERTEX = 2;
#ifndef GL_ES
        glVertexAttribIPointer(attributeIds[2], BONES_PER_VERTEX, GL_INT, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#else
        glVertexAttribPointer(attributeIds[2], BONES_PER_VERTEX, GL_INT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#endif
        glVertexAttribPointer(attributeIds[3], BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2 + sizeof(int)*BONES_PER_VERTEX));
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);
}

int VBO::primitivesCount() {
	return this->primitivesCount_;
}

} // namespace lau
