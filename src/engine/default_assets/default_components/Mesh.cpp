#include "Factories.hpp"
#include "Mesh.hpp"
#include "utils/IO.h"
#include "LauCommon.h"

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
        int indexRemainders[8],
        Mesh::FacePrimitive& primitive,
        int& primitiveSize,
        bool& hasFaceMaterial,
        bool& hasFaceUV,
        bool& hasVertexUVs,
        bool& hasFaceNormal,
        bool& hasVertexNormals,
        bool& hasFaceColor,
        bool& hasVertexColors) {

    primitive = (facesFormat&0x01)==0?Mesh::Triangle:Mesh::Quad;
    hasFaceMaterial = (facesFormat&0x02)!=0;
    hasFaceUV = (facesFormat&0x04)!=0;
    hasVertexUVs = (facesFormat&0x08)!=0;
    hasFaceNormal = (facesFormat&0x10)!=0;
    hasVertexNormals = (facesFormat&0x20)!=0;
    hasFaceColor = (facesFormat&0x40)!=0;
    hasVertexColors = (facesFormat&0x80)!=0;

    primitiveSize = (primitive == Mesh::Triangle) ? 3 : 4;

    indexRemainders[0] = primitiveSize;
    indexRemainders[1] = indexRemainders[0] + (hasFaceMaterial ? 1 : 0);
    // You either have face uv or vertex uvs (or none, but never both)
    indexRemainders[2] = indexRemainders[1] + (hasFaceUV ? 1 : 0);
    indexRemainders[3] = indexRemainders[1] + (hasVertexUVs ? primitiveSize : 0);
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
        meshFile.begin()->second.push_back('\0');
        rapidjson::Document serializedMesh;
        serializedMesh.Parse((char*)(meshFile.begin()->second.data()));

        vector<float> cachedVertices;
        vector<float> cachedNormals;
        vector<int> cachedFaces;

        cachedVertices.reserve(serializedMesh["vertices"].Size());
        cachedFaces.reserve(serializedMesh["faces"].Size());

        // Grab vertices
        const Value& _verts=serializedMesh["vertices"];
        for (Value::ConstValueIterator itr = _verts.Begin();
                itr != _verts.End(); ++itr) {
            cachedVertices.push_back(itr->GetDouble());
        }

        // Grab normals
        const Value& _norms=serializedMesh["normals"];
        for (Value::ConstValueIterator itr = _norms.Begin();
                itr != _norms.End(); ++itr) {
            cachedNormals.push_back(itr->GetDouble());
        }

        // Grab faces
        const Value& _inds=serializedMesh["faces"];
        
        // Cache indices
        for (Value::ConstValueIterator facesItr = _inds.Begin();
                facesItr != _inds.End(); ++facesItr) {
            cachedFaces.push_back(facesItr->GetInt());
        }

        // Reassemble mesh into an internal, VBO friendly format
        FacePrimitive primitive;
        int primitiveSize;
        int indexRemainders[8];
        bool hasFaceMaterial, hasFaceUV, hasVertexUVs, hasFaceNormal,
             hasVertexNormals, hasFaceColor, hasVertexColors;
        uint8_t facesFormat = static_cast<uint8_t>(_inds[0].GetInt());

        computeFaceParameters(facesFormat, indexRemainders, primitive,
                                primitiveSize, hasFaceMaterial, hasFaceUV,
                                hasVertexUVs, hasFaceNormal, hasVertexNormals,
                                hasFaceColor, hasVertexColors);

        int faceStep = indexRemainders[7] + 1;

        vector<float> vertices;
        vector<float> normals;
        vector<int> faces;
        int currentIndex = 0;
        const int DIMS = 3;
        const int PRIMITIVE_SIZE = 3;

        for(int i = 1; i < cachedFaces.size(); i+= faceStep) {
            facesFormat = static_cast<uint8_t>(cachedFaces[i-1]);

            // Reassemble mesh into an internal, VBO friendly format
            computeFaceParameters(facesFormat, indexRemainders, primitive,
                                   primitiveSize, hasFaceMaterial, hasFaceUV,
                                   hasVertexUVs, hasFaceNormal, hasVertexNormals,
                                   hasFaceColor, hasVertexColors);

            faceStep = indexRemainders[7]+1;


            for(int v = 0; v < PRIMITIVE_SIZE; ++v) {
                for(int d = 0; d < DIMS; ++d) {
                    vertices.push_back(cachedVertices[cachedFaces[i+v]*DIMS + d]);
                }
                faces.push_back(currentIndex);
                currentIndex++;
            }
            if(primitive == Quad) {
                // Transform the quad into two triangles by pushing the fourth
                // vertex and adding the remaining ones to the faces list
                for(int d = 0; d < DIMS; ++d) {
                    vertices.push_back(cachedVertices[cachedFaces[i+PRIMITIVE_SIZE]*DIMS + d]);
                }
                faces.push_back(currentIndex-3);
                faces.push_back(currentIndex-1);
                faces.push_back(currentIndex);
                currentIndex++;
            }

            if(hasVertexNormals) {
                for(int n = 0; n < PRIMITIVE_SIZE; ++n) {
                    for(int d = 0; d < DIMS; ++d) {
                        normals.push_back(cachedNormals[cachedFaces[i+indexRemainders[4]+n]*DIMS + d]);
                    }
                }
                if(primitive == Quad) {
                    for(int d = 0; d < DIMS; ++d) {
                        normals.push_back(cachedNormals[cachedFaces[i+indexRemainders[4]+PRIMITIVE_SIZE]*DIMS + d]);
                    }
                }
            } else if(hasFaceNormal) {
                for(int n = 0; n < primitiveSize; ++n) {
                    for(int d = 0; d < DIMS; ++d) {
                        normals.push_back(cachedNormals[cachedFaces[i+indexRemainders[4]]*DIMS + d]);
                    }
                }
            }
        }

        // Initialize VBO
        vbo = shared_ptr<VBO>(new VBO(DIMS, vertices, normals, faces));
    }
}

//////
// Factory
#define MESH_ID 1
__LAU_CREATE_COMPONENT_INITIALIZER(lau::Mesh, MESH_ID)

} // namespace lau
