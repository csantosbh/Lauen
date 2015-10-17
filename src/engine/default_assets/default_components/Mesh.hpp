#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <map>

#include <rapidjson/document.h>
#include <Eigen/Eigen>

#include "LauCommon.h"
#include "Component.hpp"
#include "utils/Callback.hpp"

namespace lau {

class Animation {
public:
    struct Keyframe {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        Eigen::Vector3f position;
        Eigen::Quaternionf rotation;
        Eigen::Vector3f scale;
        float time;
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
    void load(const std::string& path);
    bool isLoaded();
    Callback<> onLoad;

    const std::map<std::string, Animation>& getAnimations() const;
    const std::vector<int>& getBoneParents() const;
    const std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>>& getBonePoses() const;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<int> faces;
    std::vector<int> skinIndices;
    std::vector<float> skinWeights;

private:
    enum FacePrimitive { Triangle, Quad };

    // Animation related data
    std::vector<int> boneParents_;
    bool isLoaded_;
    std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> bonePoses_;
    std::map<std::string, Animation> animations_;

    void onLoadJsonMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile, std::string fname);
    void processLoadedMesh(std::deque<std::pair<bool, std::vector<uint8_t>>>& meshFile);

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
