#include "Game.hpp"
#include "Factories.hpp"
#include "Mesh.hpp"
#include "utils/IO.h"
#include "utils/Time.h"
#include "LauCommon.h"
#include "utils/ThreadPool.hpp"
#ifdef JAVASCRIPT
#include <emscripten.h>
#endif

using namespace std;
using namespace rapidjson;
using namespace Eigen;

namespace lau {

Mesh::Mesh() {
    // TODO the default constructor must not load any mesh
    utils::IO::getInstance().requestFiles({
        "default_assets/default_components/primitive_meshes/Plane.json"
    }, std::bind(&Mesh::onLoadJsonMesh, this, std::placeholders::_1, "default_assets/default_components/primitive_meshes/Plane.json"));
}

Mesh::Mesh(const rapidjson::Value& fields) {
    string meshName = fields["mesh"].GetString();
    const string supportedFormats[] = {".json"};

    if(meshName.length() >= supportedFormats[0].length() &&
        meshName.compare(meshName.length()-supportedFormats[0].length(),
                         supportedFormats[0].length(),
                         supportedFormats[0]) == 0) {
        utils::IO::getInstance().requestFiles({
            meshName
        }, std::bind(&Mesh::onLoadJsonMesh, this, std::placeholders::_1, meshName));
    }
}

void Mesh::update(float dt) {
}

VBO* Mesh::getVBO() {
	return this->vbo.get();
}

const std::map<std::string, Animation>& Mesh::getAnimations() const {
    return animations_;
}

const std::vector<int>& Mesh::getBoneParents() const {
    return boneParents_;
}

const std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>>& Mesh::getBonePoses() const {
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

void Mesh::onLoadJsonMesh(deque<pair<bool, vector<uint8_t>>>& meshFile, string fname) {
    if(!meshFile.begin()->first) {
        lerr << "[Mesh] Error loading mesh file \"" << fname << "\"" << endl;
    } else {
        double a = utils::time::now();
        auto f = bind(&Mesh::processLoadedMesh, this, meshFile);
        double b = utils::time::now();
        lout << (b-a) << "s elapsed" << endl;
        ThreadPool::startJob(f);
        a = utils::time::now();
        lout << (a-b) << "s on thread" << endl;
    }
}

void Mesh::grabAnimation(const rapidjson::Value& serializedAnim) {
    Animation animation;
    animation.name = serializedAnim["name"].GetString();
    animation.fps = serializedAnim["fps"].GetDouble();
    animation.length = serializedAnim["length"].GetDouble();

    const Value& _bones = serializedAnim["hierarchy"];
    // TODO provavelmente o membro parent nao era pra significar meu id! nesse caso, mudar a variavel teste pra algo decente nesse for.
    int teste = 0;
    for(Value::ConstValueIterator boneItr = _bones.Begin();
                    boneItr != _bones.End(); ++boneItr) {

        const Value& _keys = (*boneItr)["keys"];
        int boneId = teste++;//(*boneItr)["parent"].GetInt()+1;

        assert((*boneItr)["parent"].GetInt() < boneId);

        vector<Animation::Keyframe> boneKeyframes;
        for(Value::ConstValueIterator keyItr = _keys.Begin();
                keyItr != _keys.End(); ++keyItr) {
            Animation::Keyframe key;

            // Translation
            if(keyItr->HasMember("pos")) {
                assert((*keyItr)["pos"].Size()==3);
                key.position = Vector3f((*keyItr)["pos"][0].GetDouble(),
                        (*keyItr)["pos"][1].GetDouble(),
                        (*keyItr)["pos"][2].GetDouble());
            } else {
                //key.position = Vector3f::Zero();
                key.position = boneKeyframes.back().position;
            }

            // Rotation
            if(keyItr->HasMember("rot")) {
                assert((*keyItr)["rot"].Size()==4);
                key.rotation = Quaternionf((*keyItr)["rot"][3].GetDouble(),
                        (*keyItr)["rot"][0].GetDouble(),
                        (*keyItr)["rot"][1].GetDouble(),
                        (*keyItr)["rot"][2].GetDouble());
            } else {
                assert(!keyItr->HasMember("rotq"));
                //key.rotation = Quaternionf::Identity();
                key.rotation = boneKeyframes.back().rotation;
            }

            // Scale
            if(keyItr->HasMember("scl")) {
                assert((*keyItr)["scl"].Size()==3);
                key.scale = Vector3f((*keyItr)["scl"][0].GetDouble(),
                        (*keyItr)["scl"][1].GetDouble(),
                        (*keyItr)["scl"][2].GetDouble());
            } else {
                //key.scale = Vector3f::Ones();
                key.scale = boneKeyframes.back().scale;
            }

            // Time
            key.time = (*keyItr)["time"].GetDouble();

            boneKeyframes.push_back(key);
        }
        animation.boneKeyframes[boneId] = boneKeyframes;
    }

    animations_[animation.name] = animation;
}

void Mesh::processLoadedMesh(deque<pair<bool, vector<uint8_t>>>& meshFile) {
    meshFile.begin()->second.push_back('\0');
    rapidjson::Document serializedMesh;
    serializedMesh.Parse((char*)(meshFile.begin()->second.data()));

    vector<float> cachedVertices;
    vector<float> cachedNormals;
    vector<int> cachedSkinIndices;
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

    if(serializedMesh["metadata"]["normals"].GetInt() > 0) {
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
                cachedSkinIndices.push_back(_skinIdx[s_i].GetInt());
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
                cachedSkinIndices.push_back(cachedSkinIndices.back());
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
            Vector3f t;
            if(itr->HasMember("pos")) {
                assert((*itr)["pos"].Size()==3);
                t = Vector3f((*itr)["pos"][0].GetDouble(),
                        (*itr)["pos"][1].GetDouble(),
                        (*itr)["pos"][2].GetDouble());
            } else {
                t = Vector3f::Zero();
            }

            // Rotation
            // TODO pode ter erro aqui: quat idx 0 vs 3
            Quaternionf r;
            if(itr->HasMember("rotq")) {
                assert((*itr)["rotq"].Size()==4);
                r = Quaternionf((*itr)["rotq"][3].GetDouble(),
                        (*itr)["rotq"][0].GetDouble(),
                        (*itr)["rotq"][1].GetDouble(),
                        (*itr)["rotq"][2].GetDouble());
            } else {
                r = Quaternionf::Identity();
            }

            // Scale
            Vector3f s;
            if(itr->HasMember("scl")) {
                assert((*itr)["scl"].Size()==3);
                s = Vector3f((*itr)["scl"][0].GetDouble(),
                        (*itr)["scl"][1].GetDouble(),
                        (*itr)["scl"][2].GetDouble());
            } else {
                s = Vector3f::Ones();
            }

            // Bone pose Affine matrix
            Matrix4f M;
            M.block<3,3>(0,0) = r.matrix();
            M.block<3,1>(0,3) = t;
            float* ptr = M.data();

            // Multiply by scale. This is equivalent to performing Affine = R*S.
            ptr[0] *= s[0]; ptr[4] *= s[1]; ptr[8] *= s[2];
            ptr[1] *= s[0]; ptr[5] *= s[1]; ptr[9] *= s[2];
            ptr[2] *= s[0]; ptr[6] *= s[1]; ptr[10] *= s[2];
            ptr[3] = ptr[7] = ptr[11] = 0.0;
            ptr[15] = 1.0f;
            // TODO inverter na mao!

#ifdef DEBUG
            if(boneParents_.back() >= static_cast<int>(boneParents_.size()-1)) {
                lout << "Requested parent " << boneParents_.back() << ", but I am at index " << boneParents_.size()-1 << endl;
                exit(0);
            }
#endif

            if(boneParents_.back() >= 0)
                bonePoses_.push_back(M.inverse()*bonePoses_[boneParents_.back()]);
            else
                bonePoses_.push_back(M.inverse());
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
    for(int i = 1; i < serializedFaces.Size(); i+= faceStep) {
        facesFormat = static_cast<uint8_t>(serializedFaces[i-1].GetInt());

        // Reassemble mesh into an internal, VBO friendly format
        computeFaceParameters(facesFormat, indexRemainders, primitive,
                primitiveSize, hasFaceMaterial, hasFaceUV,
                hasVertexUVs, hasFaceNormal, hasVertexNormals,
                hasFaceColor, hasVertexColors);

        faceStep = indexRemainders[7]+1;

        for(int v = 0; v < PRIMITIVE_SIZE; ++v) {
            for(int d = 0; d < DIMS; ++d) {
                vertices_.push_back(cachedVertices[serializedFaces[i+v].GetInt()*DIMS + d]);
            }
            faces_.push_back(currentIndex);
            currentIndex++;

            if(modelHasAnimation) {
                // Prepare skin data
                for(int s = 0; s < BONES_PER_VERTEX; ++s) {
                    skinIndices_.push_back(cachedSkinIndices[serializedFaces[i+v].GetInt()*BONES_PER_VERTEX + s]);
                    skinWeights_.push_back(cachedSkinWeights[serializedFaces[i+v].GetInt()*BONES_PER_VERTEX + s]);
                }
            }
        }
        if(primitive == Quad) {
            // Transform the quad into two triangles by pushing the fourth
            // vertex and adding the remaining ones to the faces list
            for(int d = 0; d < DIMS; ++d) {
                vertices_.push_back(cachedVertices[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*DIMS + d]);
            }
            faces_.push_back(currentIndex-3);
            faces_.push_back(currentIndex-1);
            faces_.push_back(currentIndex);
            currentIndex++;

            if(modelHasAnimation) {
                // Prepare skin data
                for(int s = 0; s < BONES_PER_VERTEX; ++s) {
                    skinIndices_.push_back(cachedSkinIndices[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*BONES_PER_VERTEX + s]);
                    skinWeights_.push_back(cachedSkinWeights[serializedFaces[i+PRIMITIVE_SIZE].GetInt()*BONES_PER_VERTEX + s]);
                }
            }
        }

        if(hasVertexNormals) {
            for(int n = 0; n < primitiveSize; ++n) {
                for(int d = 0; d < DIMS; ++d) {
                    normals_.push_back(cachedNormals[serializedFaces[i+indexRemainders[4]+n].GetInt()*DIMS + d]);
                }
            }
        } else if(hasFaceNormal) {
            for(int n = 0; n < primitiveSize; ++n) {
                for(int d = 0; d < DIMS; ++d) {
                    normals_.push_back(cachedNormals[serializedFaces[i+indexRemainders[4]].GetInt()*DIMS + d]);
                }
            }
        } else {
            // Compute normals from face vertices
            int n = vertices_.size()/DIMS-3;
            Vector3f p0(vertices_[n*DIMS],
                        vertices_[n*DIMS+1],
                        vertices_[n*DIMS+2]);
            Vector3f p1(vertices_[(n+1)*DIMS],
                        vertices_[(n+1)*DIMS+1],
                        vertices_[(n+1)*DIMS+2]);
            Vector3f p2(vertices_[(n+2)*DIMS],
                        vertices_[(n+2)*DIMS+1],
                        vertices_[(n+2)*DIMS+2]);
            Vector3f normal = (p1-p0).cross(p2-p0).normalized();
            for(int n = 0; n < primitiveSize; ++n) {
                normals_.push_back(normal[0]);
                normals_.push_back(normal[1]);
                normals_.push_back(normal[2]);
            }
        }
    }

    // Initialize VBO. Since it plays with the OpenGL context (which is not
    // thread safe), it must take place in the main thread.
    Game::scheduleMainThreadTask([this, modelHasAnimation]() {
        if(modelHasAnimation) {
            vbo = shared_ptr<VBO>(new VBO(DIMS, vertices_, normals_, faces_, skinIndices_, skinWeights_));
        }
        else {
            vbo = shared_ptr<VBO>(new VBO(DIMS, vertices_, normals_, faces_));
        }

        vertices_.clear();
        normals_.clear();
        faces_.clear();
        skinIndices_.clear();
        skinWeights_.clear();
    });
}

//////
// Factory
#define MESH_ID 1
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Mesh, MESH_ID)

} // namespace lau
