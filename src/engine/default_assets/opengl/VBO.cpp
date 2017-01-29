#include <cassert>
#include <stdio.h>
#include <sstream>
#include "VBO.hpp"

using namespace std;
namespace lau {

VBO::VBO() : dimensions_(0), primitivesCount_(0), bufferFormat_(static_cast<VBOFormat>(0)) {
	primitivesCount_ = 0;
}

VBO::~VBO() {
    glDeleteBuffers(2, bufferIds_);
}

void VBO::create(uint8_t dimensions, vector<float>& vertices, vector<int>& indices) {
    if(bufferFormat_ != 0) {
        glDeleteBuffers(2, bufferIds_);
    }

    dimensions_ = dimensions;
    primitivesCount_ = indices.size();
    bufferFormat_ = VBOFormat::Vf;
    stride_= sizeof(float) * dimensions_;

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
}

void VBO::create(uint8_t dimensions, vector<float>& vertices, vector<float>& normals, vector<int>& indices) {
    if(bufferFormat_ != 0) {
        glDeleteBuffers(2, bufferIds_);
    }

    dimensions_ = dimensions;
    primitivesCount_ = indices.size();
    bufferFormat_ = VBOFormat::VNf;

    assert(vertices.size() == normals.size());
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
    int sentinel = static_cast<int>(vertices.size());
    for(int i = 0; i < sentinel; i += dimensions_) {
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

    const int numComponents_ = 2;
    stride_ = sizeof(float) * dimensions_ * numComponents_;
}

void VBO::create(uint8_t dimensions, vector<float>& vertices, vector<float>& normals, vector<int>& indices, vector<unsigned short>& skinIndices, vector<float>& skinWeights) {
    if(bufferFormat_ != 0) {
        glDeleteBuffers(2, bufferIds_);
    }

    dimensions_ = dimensions;
    primitivesCount_ = indices.size();
    bufferFormat_ = VBOFormat::VNfSkinIW;

    assert(vertices.size() == normals.size());
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
    modelBuffer.reserve(vertices.size()*sizeof(float)+normals.size()*sizeof(float)+BONES_PER_VERTEX*sizeof(unsigned short)+BONES_PER_VERTEX*sizeof(float));
	int nVerts = static_cast<int>(vertices.size())/dimensions_;
    for(int i = 0; i < nVerts; ++i) {
        // Vertex
		const int Vi = i*dimensions_;
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(vertices.data()+Vi),
                reinterpret_cast<uint8_t*>(vertices.data()+Vi+dimensions_));
        // Normal
		const int Ni = Vi;
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(normals.data()+Ni),
                reinterpret_cast<uint8_t*>(normals.data()+Ni+dimensions_));

        // Skin indices
		const int Ii = i*BONES_PER_VERTEX;
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(skinIndices.data()+Ii),
                reinterpret_cast<uint8_t*>(skinIndices.data()+Ii+BONES_PER_VERTEX));
        // Skin weights
		const int Wi = Ii;
        modelBuffer.insert(modelBuffer.end(),
                reinterpret_cast<uint8_t*>(skinWeights.data()+Wi),
                reinterpret_cast<uint8_t*>(skinWeights.data()+Wi+BONES_PER_VERTEX));
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

    stride_ = modelBuffer.size()/nVerts;
}

// TODO make this generic as to support IPointer (int) and LPointer (double)
// TODO the parameter for this function should be a struct, with named fields instead of an array
void VBO::bindAttributes(const ShaderProgram& shader) {
    // Create vertex attributes so the VBO can be accessed from shaders
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);

    // TODO usar switch
    if((bufferFormat_&0x1) != 0) {
        // Vertex
        glEnableVertexAttribArray(shader.vertexAttribId);
        glVertexAttribPointer(shader.vertexAttribId, dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    }
    if((bufferFormat_&0x2) != 0) {
        // Normal
        glEnableVertexAttribArray(shader.normalAttribId);
        glVertexAttribPointer(shader.normalAttribId, dimensions_, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_));
    }
    if((bufferFormat_&0x4) != 0) {
        // Skin index&weight
        const int BONES_PER_VERTEX = 2;
        glEnableVertexAttribArray(shader.skinIndexAttribId);
        glEnableVertexAttribArray(shader.skinWeightAttribId);
#ifndef GL_ES
        glVertexAttribIPointer(shader.skinIndexAttribId, BONES_PER_VERTEX, GL_UNSIGNED_SHORT, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#else
        glVertexAttribPointer(shader.skinIndexAttribId, BONES_PER_VERTEX, GL_UNSIGNED_SHORT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#endif
        glVertexAttribPointer(shader.skinWeightAttribId, BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2 + sizeof(unsigned short)*BONES_PER_VERTEX));
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

void VBO::bindForDrawing(const ShaderProgram& shader) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferIds_[0]);

    if((bufferFormat_&0x1) != 0) {
        glVertexAttribPointer(shader.vertexAttribId, dimensions_, GL_FLOAT, GL_FALSE, stride_, 0);
    }
    if((bufferFormat_&0x2) != 0) {
        glVertexAttribPointer(shader.normalAttribId, dimensions_, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_));
    }
    if((bufferFormat_&0x4) != 0) {
        // Skin index&weight
        const int BONES_PER_VERTEX = 2;
#ifndef GL_ES
        glVertexAttribIPointer(shader.skinIndexAttribId, BONES_PER_VERTEX, GL_UNSIGNED_SHORT, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#else
        glVertexAttribPointer(shader.skinIndexAttribId, BONES_PER_VERTEX, GL_UNSIGNED_SHORT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2));
#endif
        glVertexAttribPointer(shader.skinWeightAttribId, BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, stride_, (GLvoid*)(sizeof(float)*dimensions_*2 + sizeof(unsigned short)*BONES_PER_VERTEX));
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds_[1]);
}

int VBO::primitivesCount() {
	return this->primitivesCount_;
}

} // namespace lau
