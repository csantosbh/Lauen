#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <map>

#include <rapidjson/document.h>
#include <Eigen/Eigen>

#include "LauCommon.h"
#include "opengl/VBO.h"
#include "Component.hpp"

namespace lau {

class Animation {
public:
    struct Keyframe {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        Eigen::Vector3f position;
        Eigen::Quaternionf rotation;
        Eigen::Vector3f scale;
        float time;
        int tmpParent;
    };

    std::string name;
    float fps;
    float length;
    // TODO this is bones[keyframes]. This is not cache-optimal. The better way would be to make the structure below keyframes[bones]. But this will require normalizing all animations (make them have the same frequencies -- even though they may not have the same durations).
    std::map<int, std::vector<Keyframe>> boneKeyframes;
};

class Mesh : public Component {
public:
    Mesh();
	Mesh(const rapidjson::Value& fields);

	void update(float dt);
    VBO* getVBO();

    const std::map<std::string, Animation>& getAnimations() const;
    const std::vector<int>& getBoneParents() const;
    const std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>>& getBonePoses() const;

private:
    enum FacePrimitive { Triangle, Quad };
    std::vector<float> vertices_;
    std::vector<float> normals_;
    std::vector<int> faces_;
    std::vector<int> skinIndices_;
    std::vector<float> skinWeights_;

    // Animation related data
    std::vector<int> boneParents_;
    std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> bonePoses_;
    std::map<std::string, Animation> animations_;

    void onLoadJsonMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile, std::string fname);
    void processLoadedMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile);

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
    void grabAnimation(const rapidjson::Value& serializedAnim);
};

} // namespace
