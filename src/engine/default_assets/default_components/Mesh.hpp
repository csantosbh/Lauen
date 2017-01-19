#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <map>

#include <rapidjson/document.h>

#include "LauCommon.h"
#include "Component.hpp"
#include "utils/Callback.hpp"

#include "math/Vector.hpp"
#include "math/Matrix.hpp"
#include "math/Quaternion.hpp"

namespace lau {

class Animation {
public:
    struct Keyframe {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        math::vec3 position;
        math::quaternion rotation;
        math::vec3 scale;
        float time;
    };

    std::string name;
    float fps;
    float length;
    // TODO this is bones[keyframes]. This is not cache-optimal. The better way would be to make the structure below keyframes[bones]. But this will require normalizing all animations (make them have the same frequencies -- even though they may not have the same durations).
    std::vector<std::vector<Keyframe>> boneKeyframes;
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
    const std::vector<math::mat4, Eigen::aligned_allocator<math::mat4>>& getBonePoses() const;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<int> faces;
    std::vector<unsigned short> skinIndices;
    std::vector<float> skinWeights;

private:
    enum FacePrimitive { Triangle, Quad };

    // Animation related data
    std::vector<int> boneParents_;
    bool isLoaded_;
    std::vector<math::mat4, Eigen::aligned_allocator<math::mat4>> bonePoses_;
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
