#include "Mesh.hpp"
#include "utils/IO.h"

using namespace std;
using namespace rapidjson;

namespace lau {

Mesh::Mesh(const rapidjson::Value& fields) {
    utils::IO::getInstance().requestFiles({
        fields["mesh"].GetString()
    }, std::bind(&Mesh::onLoadMesh, this, std::placeholders::_1, fields["mesh"].GetString()));
}

void Mesh::update(float dt) {
}

VBO& Mesh::getVBO() {
	return *this->vbo.get();
}

void Mesh::onLoadMesh(deque<pair<bool, vector<uint8_t>>>& meshFile, string fname) {
    if(!meshFile.begin()->first) {
        lout << "[Mesh] Error loading mesh file \"" << fname << "\"" << endl;
    } else {
        meshFile.begin()->second.push_back('\0');
        rapidjson::Document serializedMesh;
        serializedMesh.Parse((char*)(meshFile.begin()->second.data()));

        vector<float> vertices;
        vector<int> indices;

        vertices.reserve(serializedMesh["vertices"].Size());
        indices.reserve(serializedMesh["indices"].Size());

        // Grab vertices
        const Value& _verts=serializedMesh["vertices"];
        for (Value::ConstValueIterator itr = _verts.Begin();
                itr != _verts.End(); ++itr) {
            vertices.push_back(itr->GetDouble());
        }

        // Grab indices
        const Value& _inds=serializedMesh["indices"];
        for (Value::ConstValueIterator itr = _inds.Begin();
                itr != _inds.End(); ++itr) {
            indices.push_back(itr->GetInt());
        }

        // Initialize VBO
        vbo = shared_ptr<VBO>(new VBO(3, vertices, indices));
    }
}

} // namespace lau