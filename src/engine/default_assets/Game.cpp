#include "Game.hpp"
#include "Factories.hpp"
#include "utils/IO.h"
#include "LauCommon.h"

namespace lau {

using namespace std;

void Game::init(int windowWidth, int windowHeight) {
    glClearColor(0,0,0,1);
    glViewport(0, 0, windowWidth, windowHeight);

    // Load game objects
    utils::IO::getInstance().requestFiles({"scenes/scene0.json"}, [this] (deque<pair<bool, vector<uint8_t>>>& fileData) {
        fileData.begin()->second.push_back('\0');
        rapidjson::Document serializedGameObjects;
        serializedGameObjects.Parse((char*)(fileData.begin()->second.data()));
        gameObjects = Factories::gameObjectFactory(serializedGameObjects);
    });
}

void Game::resize(int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
}

void Game::draw(double temporalAlpha) {
    glClear(GL_COLOR_BUFFER_BIT);
    for(auto& gameObject: gameObjects) {
        gameObject->draw(temporalAlpha);
    }
}

void Game::update(double dt) {
    for(auto& gameObject: gameObjects) {
        gameObject->update(dt);
    }
}

void Game::terminate() {
}

} // namespace lau
