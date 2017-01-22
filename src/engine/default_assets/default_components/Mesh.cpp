#include "Game.hpp"
#include "Factories.hpp"
#include "Mesh.hpp"
#include "utils/IO.h"
#include "utils/Time.h"
#include "LauCommon.h"
#include "utils/ThreadPool.hpp"
#include "Transform.hpp"

#include "math/Matrix.hpp"

#ifdef JAVASCRIPT
#include <emscripten.h>
#endif

using namespace std;
using namespace rapidjson;
using namespace lau::math;

namespace lau {

Mesh::Mesh() : isLoaded_(true) {
}

Mesh::Mesh(const rapidjson::Value& fields) : isLoaded_(true) {
    string meshName = fields["mesh"].GetString();
    load(meshName);
}

void Mesh::update(float dt) {
}

void Mesh::load(const string& path) {
    isLoaded_ = false;
    const string supportedFormats[] = {".lmf"};
    if(path.length() >= supportedFormats[0].length() &&
            path.compare(path.length()-supportedFormats[0].length(),
                supportedFormats[0].length(),
                supportedFormats[0]) == 0) {
        utils::IO::getInstance().requestFiles({
                path
                }, std::bind(&Mesh::onLoadJsonMesh, this, std::placeholders::_1, path));
    }
}

bool Mesh::isLoaded() {
    return isLoaded_;
}

const std::map<std::string, Animation>& Mesh::getAnimations() const {
    return animations_;
}

const std::vector<int>& Mesh::getBoneParents() const {
    return boneParents_;
}

const std::vector<mat4, Eigen::aligned_allocator<mat4>>& Mesh::getBonePoses() const {
    return bonePoses_;
}

void Mesh::computeFaceParameters(
        uint8_t facesFormat,
        int* indexRemainders,
        Mesh::FacePrimitive& primitive,
        int& primitiveSize,
        bool& hasFaceMaterial,
        bool& hasFaceUV,
        bool& hasVertexUVs,
        bool& hasFaceNormal,
        bool& hasVertexNormals,
        bool& hasFaceColor,
        bool& hasVertexColors) {

    primitive = ((facesFormat&0x01u)==0) ? Mesh::Triangle:Mesh::Quad;
    hasFaceMaterial = (facesFormat&0x02)!=0;
    hasFaceUV = (facesFormat&0x04u)!=0;
    hasVertexUVs = (facesFormat&0x08u)!=0;
    hasFaceNormal = (facesFormat&0x10u)!=0;
    hasVertexNormals = (facesFormat&0x20u)!=0;
    hasFaceColor = (facesFormat&0x40u)!=0;
    hasVertexColors = (facesFormat&0x80u)!=0;

    primitiveSize = (primitive == Mesh::Triangle) ? 3 : 4;

    indexRemainders[0] = primitiveSize;
    indexRemainders[1] = indexRemainders[0] + (hasFaceMaterial ? 1 : 0);
    // You either have face uv or vertex uvs (or none, but never both)
    indexRemainders[2] = indexRemainders[1] + (hasFaceUV ? 1 : 0);
    indexRemainders[3] = indexRemainders[2] + (hasVertexUVs ? primitiveSize : 0);
    // You either have face normal or vertex normals (or none, but never both)
    indexRemainders[4] = indexRemainders[3] + (hasFaceNormal ? 1 : 0);
    indexRemainders[5] = indexRemainders[4] + (hasVertexNormals ? primitiveSize : 0);
    // You either have face color or vertex colors (or none, but never both)
    indexRemainders[6] = indexRemainders[5] + (hasFaceColor ? 1 : 0);
    indexRemainders[7] = indexRemainders[6] + (hasVertexColors ? primitiveSize : 0);
}

void Mesh::grabAnimation(const rapidjson::Value& serializedAnim) {
    Animation animation;
    animation.name = serializedAnim["name"].GetString();
    animation.fps = serializedAnim["fps"].GetDouble();
    animation.length = serializedAnim["length"].GetDouble();

    const Value& _bones = serializedAnim["hierarchy"];
    for(Value::ConstValueIterator boneItr = _bones.Begin();
                    boneItr != _bones.End(); ++boneItr) {

        const Value& _keys = (*boneItr)["keys"];

        assert((*boneItr)["parent"].GetInt() < static_cast<int>(animation.boneKeyframes.size()));

        vector<Animation::Keyframe> boneKeyframes;
        for(Value::ConstValueIterator keyItr = _keys.Begin();
                keyItr != _keys.End(); ++keyItr) {
            Animation::Keyframe key;

            // Translation
            if(keyItr->HasMember("pos")) {
                assert((*keyItr)["pos"].Size()==3);
                key.position = vec3((*keyItr)["pos"][0].GetDouble(),
                                    (*keyItr)["pos"][1].GetDouble(),
                                    (*keyItr)["pos"][2].GetDouble());
            } else {
                assert(boneKeyframes.size() > 0);
                key.position = boneKeyframes.back().position;
            }

            // Rotation
            if(keyItr->HasMember("rot")) {
                assert((*keyItr)["rot"].Size()==4);
                key.rotation = quaternion((*keyItr)["rot"][3].GetDouble(),
                                          (*keyItr)["rot"][0].GetDouble(),
                                          (*keyItr)["rot"][1].GetDouble(),
                                          (*keyItr)["rot"][2].GetDouble());
            } else {
                assert(!keyItr->HasMember("rotq"));
                assert(boneKeyframes.size() > 0);
                key.rotation = boneKeyframes.back().rotation;
            }

            // Scale
            if(keyItr->HasMember("scl")) {
                assert((*keyItr)["scl"].Size()==3);
                key.scale = vec3((*keyItr)["scl"][0].GetDouble(),
                                 (*keyItr)["scl"][1].GetDouble(),
                                 (*keyItr)["scl"][2].GetDouble());
            } else {
                assert(boneKeyframes.size() > 0);
                key.scale = boneKeyframes.back().scale;
            }

            // Time
            key.time = (*keyItr)["time"].GetDouble();

            boneKeyframes.push_back(key);
        }
        animation.boneKeyframes.push_back(boneKeyframes);
    }

    animations_[animation.name] = animation;
}

void Mesh::onLoadJsonMesh(deque<pair<bool, vector<uint8_t>>>& meshFile, string fname) {
    if(!meshFile.begin()->first) {
        lerr << "[Mesh] Error loading mesh file \"" << fname << "\"" << endl;
    } else {
        meshFile.begin()->second.push_back('\0');
        rapidjson::Document serializedMesh;
        serializedMesh.Parse((char*)(meshFile.begin()->second.data()));

        vector<float> cachedVertices;
        vector<float> cachedNormals;
        vector<unsigned short> cachedSkinIndices;
        vector<float> cachedSkinWeights;

        const int DIMS = 3;
        const int PRIMITIVE_SIZE = 3;
        const int BONES_PER_VERTEX = 2;

        cachedVertices.reserve(serializedMesh["vertices"].Size());
        int nVerts = serializedMesh["vertices"].Size()/DIMS;

        bool modelHasAnimation = false;

        // Grab vertices
        const Value& _verts=serializedMesh["vertices"];
        for (Value::ConstValueIterator itr = _verts.Begin();
                itr != _verts.End(); ++itr) {
            cachedVertices.push_back(static_cast<float>(itr->GetDouble()));
        }

        if(serializedMesh["metadata"].HasMember("normals") && serializedMesh["metadata"]["normals"].GetInt() > 0) {
            // Grab normals
            const Value& _norms=serializedMesh["normals"];
            for (Value::ConstValueIterator itr = _norms.Begin();
                    itr != _norms.End(); ++itr) {
                cachedNormals.push_back(static_cast<float>(itr->GetDouble()));
            }
        }

        if(serializedMesh.HasMember("skinIndices")) {
            modelHasAnimation = true;

            // Grab skin indices/weights
            int modelBonesPerVertex = serializedMesh["skinIndices"].Size() / nVerts;
            cachedSkinIndices.reserve(BONES_PER_VERTEX*nVerts);
            cachedSkinWeights.reserve(BONES_PER_VERTEX*nVerts);
            const Value& _skinIdx = serializedMesh["skinIndices"];
            const Value& _skinWs = serializedMesh["skinWeights"];

            for (int v = 0; v < nVerts; ++v) {
                int upperLim = min(modelBonesPerVertex, BONES_PER_VERTEX);
                float sumWeights = 0.0f;
                for(int s = 0; s < upperLim; ++s) {
                    int s_i = v*modelBonesPerVertex + s;
                    // Indices
                    cachedSkinIndices.push_back(static_cast<unsigned short>(_skinIdx[s_i].GetInt()));
                    // Weights
                    float w = static_cast<float>(_skinWs[s_i].GetDouble());
                    sumWeights += w;
                    cachedSkinWeights.push_back(w);
                }
                // When the model has fewer bones per vertices than we actually
                // support, we must fill the remaining indices. I select the last
                // index to at least take advantage of data locality, even though
                // we wont be using the fetched data from the uniform buffer.
                for(int s = 0; s < BONES_PER_VERTEX-upperLim; ++s) {
                    cachedSkinIndices.push_back(static_cast<unsigned short>(cachedSkinIndices.back()));
                    cachedSkinWeights.push_back(0.0f);
                }

                // Normalize weights
                if(sumWeights > 0.0f) {
                    for(int s = static_cast<int>(cachedSkinWeights.size())-1;
                            s >= static_cast<int>(cachedSkinWeights.size())-BONES_PER_VERTEX;
                            --s) {
                        assert(s>=0);
                        cachedSkinWeights[s] /= sumWeights;
                    }
                }
            }

            // Grab bones
            // TODO perform topological sorting of bones so that during rendering I can always assume that the parent has already been processed
            const Value& _bones = serializedMesh["bones"];
            const int boneCount = serializedMesh["metadata"]["bones"].GetInt();

            boneParents_.reserve(boneCount);
            bonePoses_.reserve(boneCount);

            for (Value::ConstValueIterator itr = _bones.Begin();
                    itr != _bones.End(); ++itr) {
                // Parent
                boneParents_.push_back((*itr)["parent"].GetInt());
                // Translation
                vec3 t;
                if(itr->HasMember("pos")) {
                    assert((*itr)["pos"].Size()==3);
                    t = vec3((*itr)["pos"][0].GetDouble(),
                             (*itr)["pos"][1].GetDouble(),
                             (*itr)["pos"][2].GetDouble());
                } else {
                    t = vec3(0);
                }

                // Rotation
                quaternion r;
                if(itr->HasMember("rotq")) {
                    assert((*itr)["rotq"].Size()==4);
                    r = quaternion((*itr)["rotq"][3].GetDouble(),
                                   (*itr)["rotq"][0].GetDouble(),
                                   (*itr)["rotq"][1].GetDouble(),
                                   (*itr)["rotq"][2].GetDouble());
                } else {
                    r = quaternion::identity();
                }

                // Scale
                vec3 s;
                if(itr->HasMember("scl")) {
                    assert((*itr)["scl"].Size()==3);
                    s = vec3((*itr)["scl"][0].GetDouble(),
                             (*itr)["scl"][1].GetDouble(),
                             (*itr)["scl"][2].GetDouble());
                } else {
                    s = vec3(1);
                }

                // Bone pose Affine matrix
                mat4 M;
                Transform::createInvMat4FromTransforms(t, r, s, M);

                if(boneParents_.back() >= 0)
                    bonePoses_.push_back(M*bonePoses_[boneParents_.back()]);
                else
                    bonePoses_.push_back(M);
            }

            // Grab animations
            if(serializedMesh.HasMember("animation"))
                grabAnimation(serializedMesh["animation"]);
            else {
                const Value& _anims = serializedMesh["animations"];
                for(Value::ConstValueIterator itr = _anims.Begin();
                        itr != _anims.End(); ++itr) {
                    grabAnimation(*itr);
                }
            }
        }

        // Grab faces
        const Value& serializedFaces=serializedMesh["faces"];

        // Reassemble mesh into an internal, VBO friendly format
        FacePrimitive primitive;
        int faceStep, primitiveSize;
        int indexRemainders[8];
        bool hasFaceMaterial, hasFaceUV, hasVertexUVs, hasFaceNormal,
             hasVertexNormals, hasFaceColor, hasVertexColors;
        uint8_t facesFormat;

        int currentIndex = 0;
        const int sentinel = static_cast<int>(serializedFaces.Size());
        for(int i = 1; i < sentinel; i+= faceStep) {
            facesFormat = static_cast<uint8_t>(serializedFaces[i-1].GetInt());

            // Reassemble mesh into an internal, VBO friendly format
            computeFaceParameters(facesFormat, indexRemainders, primitive,
                    primitiveSize, hasFaceMaterial, hasFaceUV,
                    hasVertexUVs, hasFaceNormal, hasVertexNormals,
                    hasFaceColor, hasVertexColors);

            faceStep = indexRemainders[7]+1;

            for(int v = 0; v < PRIMITIVE_SIZE; ++v) {
                for(int d = 0; d < DIMS; ++d) {
                    vertices.push_back(cachedVertices[serializedFaces[i+v].GetInt()*DIMS + d]);
                }
                faces.push_back(currentIndex);
                currentIndex++;

                if(modelHasAnimation) {
                    // Prepare skin data
                    for(int s = 0; s < BONES_PER_VERTEX; ++s) {
                        skinIndices.push_back(cachedSkinIndices[serializedFaces[i+v].GetInt()*BONES_PER_VERTEX + s]);
                        skinWeights.push_back(cachedSkinWeights[serializedFaces[i+v].GetInt()*BONES_PER_VERTEX + s]);
                    }
                }
            }
            if(primitive == Quad) {
                // Transform the quad into two triangles by pushing the fourth
                // vertex and adding the remaining ones to the faces list
                for(int d = 0; d < DIMS; ++d) {
                    vertices.push_back(cachedVertices[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*DIMS + d]);
                }
                faces.push_back(currentIndex-3);
                faces.push_back(currentIndex-1);
                faces.push_back(currentIndex);
                currentIndex++;

                if(modelHasAnimation) {
                    // Prepare skin data
                    for(int s = 0; s < BONES_PER_VERTEX; ++s) {
                        skinIndices.push_back(cachedSkinIndices[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*BONES_PER_VERTEX + s]);
                        skinWeights.push_back(cachedSkinWeights[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*BONES_PER_VERTEX + s]);
                    }
                }
            }

            if(hasVertexNormals) {
                for(int n = 0; n < primitiveSize; ++n) {
                    for(int d = 0; d < DIMS; ++d) {
                        normals.push_back(cachedNormals[serializedFaces[i+indexRemainders[4]+n].GetInt()*DIMS + d]);
                    }
                }
            } else if(hasFaceNormal) {
                for(int n = 0; n < primitiveSize; ++n) {
                    for(int d = 0; d < DIMS; ++d) {
                        normals.push_back(cachedNormals[serializedFaces[i+indexRemainders[4]].GetInt()*DIMS + d]);
                    }
                }
            } else {
                // Compute normals from face vertices
                int n = vertices.size()/DIMS-3;
                vec3 p0(vertices[n*DIMS],
                        vertices[n*DIMS+1],
                        vertices[n*DIMS+2]);
                vec3 p1(vertices[(n+1)*DIMS],
                        vertices[(n+1)*DIMS+1],
                        vertices[(n+1)*DIMS+2]);
                vec3 p2(vertices[(n+2)*DIMS],
                        vertices[(n+2)*DIMS+1],
                        vertices[(n+2)*DIMS+2]);
                vec3 normal = (p1-p0).cross(p2-p0).normalized();
                for(int n = 0; n < primitiveSize; ++n) {
                    normals.push_back(normal[0]);
                    normals.push_back(normal[1]);
                    normals.push_back(normal[2]);
                }
            }
        }
    }
    isLoaded_ = true;
    onLoad.broadcast();
    onLoad.clear();
}

//////
// Factory
#define MESH_ID 1
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Mesh, MESH_ID)

} // namespace lau
