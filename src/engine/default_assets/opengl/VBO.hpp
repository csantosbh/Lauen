#pragma once
#include <vector>

#include "LauCommon.h"
#include "ShaderProgram.hpp"

namespace lau {

// TODO create an EBO class
class VBO {
public:
    VBO();
    void create(uint8_t dimensions, std::vector<float>& vertices, std::vector<int>& indices);
    void create(uint8_t dimensions, std::vector<float>& vertices, std::vector<float>& normals, std::vector<int>& indices);
    void create(uint8_t dimensions, std::vector<float>& vertices, std::vector<float>& normals, std::vector<int>& indices, std::vector<int>& skinIndices, std::vector<float>& skinWeights);
	~VBO();

    void bindAttributes(const ShaderProgram& shader);
	void bindForDrawing(const ShaderProgram& shader);
	int primitivesCount();

private:
    enum VBOFormat {
        Vf = 0x1,
        VNf = 0x3,
        VNfSkinIW = 0x7,
    };

    uint8_t dimensions_;
	int primitivesCount_;
    GLuint bufferIds_[2];
    VBOFormat bufferFormat_;
    GLsizei stride_;
};

} // namespace lau
