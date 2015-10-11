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

    bool modelHasAnimation = false;

    // Grab vertices
    const Value& _verts=serializedMesh["vertices"];
    for (Value::ConstValueIterator itr = _verts.Begin();
            itr != _verts.End(); ++itr) {
        cachedVertices.push_back(static_cast<float>(itr->GetDouble()));
    }

    // Grab normals
    const Value& _norms=serializedMesh["normals"];
    for (Value::ConstValueIterator itr = _norms.Begin();
            itr != _norms.End(); ++itr) {
        cachedNormals.push_back(static_cast<float>(itr->GetDouble()));
    }

    if(serializedMesh.HasMember("skinIndices")) {
        modelHasAnimation = true;

        // Grab skin indices
        cachedSkinIndices.reserve(BONES_PER_VERTEX*cachedVertices.size());
        int modelBonesPerVertex = serializedMesh["skinIndices"].Size() / (cachedVertices.size()/DIMS);
        const Value& _skinIdx = serializedMesh["skinIndices"];
        for (int v = 0; v < cachedVertices.size(); ++v) {
            int upperLim = min(modelBonesPerVertex, BONES_PER_VERTEX);
            for(int s = 0; s < upperLim; ++s) {
                int s_i = v/DIMS*modelBonesPerVertex + s;
                cachedSkinIndices.push_back(_skinIdx[s_i].GetInt());
            }
            // When the model has fewer bones per vertices than we actually
            // support, we must fill the remaining indices. I select the last
            // index to at least take advantage of data locality, even though
            // we wont be using the fetched data from the uniform buffer.
            for(int s = 0; s < BONES_PER_VERTEX-upperLim; ++s)
                cachedSkinIndices.push_back(cachedSkinIndices.back());
        }

        // Grab skin weights
        cachedSkinWeights.reserve(BONES_PER_VERTEX*cachedVertices.size());
        const Value& _skinWs = serializedMesh["skinWeights"];
        for (int v = 0; v < cachedVertices.size(); ++v) {
            int upperLim = min(modelBonesPerVertex, BONES_PER_VERTEX);
            for(int s = 0; s < upperLim; ++s) {
                int s_i = v/DIMS*modelBonesPerVertex + s;
                cachedSkinWeights.push_back(static_cast<float>(_skinWs[s_i].GetDouble()));
            }
            // When the model has fewer bones per vertices than we actually
            // support, we must fill the remaining weights
            for(int s = 0; s < BONES_PER_VERTEX-upperLim; ++s)
                cachedSkinWeights.push_back(0.0f);
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
