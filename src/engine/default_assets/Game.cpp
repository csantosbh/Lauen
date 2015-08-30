#include "Game.hpp"
#include "Factories.hpp"
#include "utils/IO.h"
#include "LauCommon.h"
#include "default_components/Camera.hpp"
#include "window/Screen.hpp"

namespace lau {

using namespace std;

namespace lau_internal {
Game *GameInstance;
} // namespace lau_internal

Game::Game() {
}

void Game::init(int windowWidth, int windowHeight) {
    glClearColor(0,0,0,1);
    resize(windowWidth, windowHeight);

    // Load game objects
    utils::IO::getInstance().requestFiles({"scenes/scene0.json"}, [this] (deque<pair<bool, vector<uint8_t>>>& fileData) {
        fileData.begin()->second.push_back('\0');
        rapidjson::Document serializedGameObjects;
        serializedGameObjects.Parse((char*)(fileData.begin()->second.data()));
        // TODO move gameObjects_ to the GameObject class as a static field.
        // Let it manage the collection of game objects. Change the Factories
        // to comply with a friendly API for instantiating game objects so they
        // can be instantiated here.
        gameObjects_ = Factories::gameObjectFactory(serializedGameObjects);
    });
}

void Game::resize(int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
    Screen::windowResizeCallback(windowWidth, windowHeight);
}

void Game::draw(double temporalAlpha) {
    for(const auto& camera: Camera::allCameras()) {
        camera->draw(temporalAlpha);
    }
}

void Game::update(double dt) {
    for(auto& gameObject: gameObjects_) {
        gameObject->update(dt);
    }
}

void Game::terminate() {
}

} // namespace lau
