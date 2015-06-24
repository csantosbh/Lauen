#pragma once

#include <vector>
#include <deque>

#include "LauCommon.h"

#include "utils/IO.h"
#include "Factories.hpp"

namespace lau {

using namespace std;

class Game {
public:
    void init(int windowWidth, int windowHeight) {
        glClearColor(0,0,0,1);
        glViewport(0, 0, windowWidth, windowHeight);
        createSimpleGeometry();

        // Load game objects
        utils::IO::getInstance().requestFiles({"scenes/scene0.json"}, [this] (deque<pair<bool, vector<uint8_t>>>& fileData) {
            fileData.begin()->second.push_back('\0');
            rapidjson::Document serializedGameObjects;
            serializedGameObjects.Parse((char*)(&fileData.begin()->second[0]));
            gameObjects = Factories::gameObjectFactory(serializedGameObjects);
        });
    }

    void draw(double temporalAlpha) {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);

        glVertexAttribPointer(vertexAttribId, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);

        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
    }

    void update(double dt) {
        for(auto& gameObject: gameObjects) {
            gameObject->update(dt);
        }
    }

    void terminate() {
        glDeleteBuffers(2, bufferIds);
    }

private:
    vector<shared_ptr<GameObject>> gameObjects;

    ////// TODO shader&VBO&EBO classes
    //////
    GLuint bufferIds[2];
    GLuint program;
    const int vertexAttribId = 0;
    void checkShaderCompilation(GLuint shaderId) {
        GLint compilationStatus;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
        if(compilationStatus == GL_FALSE) {
            GLint logSize = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
            vector<char> infoLog(logSize);
            glGetShaderInfoLog(shaderId, logSize, &logSize, (char*)&infoLog[0]);
            // TODO proper error handling here
            lerr << (char*)&infoLog[0] << endl;
#ifndef NACL
            exit(0);
#endif
        }
    }

    void createSimpleGeometry() {
        float vertices[] = {
            -1,-1,0,
            -1,1,0,
            1,1,0,
            1,-1,0,
        };
        uint8_t indices[] = {
            0, 1, 3, 2
        };

        // Create VBO with triangle and indices
        glGenBuffers(2, bufferIds);

        // Lets upload the vertex data first.
        // Bind the buffer object to the generated vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
        // Copy the data to the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Now lets upload the indices.
        // Bind the buffer object to the generated index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[1]);
        // Upload indices to the buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Create vertex attributes
        glEnableVertexAttribArray(vertexAttribId);
        glVertexAttribPointer(vertexAttribId, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Create shaders
        utils::IO::getInstance().requestFiles({
          "default_assets/shaders/basic.vs",
          "default_assets/shaders/basic.fs"
        }, std::bind(&Game::onLoadShaders, this, std::placeholders::_1));
    }

    void onLoadShaders(deque<pair<bool, vector<uint8_t>>>&shaderFiles) {
        program = glCreateProgram();
        GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
        GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef GL_ES
        // GL ES
        // TODO pass the versions dynamically, since they will be chosen by the user in the interface
        const char version[] = "#version 100\n";
#else
        // GL core
        const char version[] = "#version 150\n";
#endif
        const GLchar *vsFull[] = {
            version,
            NULL,
        };
        const GLchar *fsFull[] = {
            version,
            NULL,
        };
        vsFull[1] = (char*)(&shaderFiles.begin()->second[0]);
        fsFull[1] = (char*)(&(++shaderFiles.begin())->second[0]);

        glShaderSource(vsId, 2, vsFull, NULL);
        glShaderSource(fsId, 2, fsFull, NULL);

        glCompileShader(vsId);
        glCompileShader(fsId);

        // Get shader compilation status
        checkShaderCompilation(vsId);
        checkShaderCompilation(fsId);

        glAttachShader(program, vsId);
        glAttachShader(program, fsId);
        glBindAttribLocation(program, vertexAttribId, "in_Position");
        glLinkProgram(program);
        glUseProgram(program);
    }
};

} // namespace lau
