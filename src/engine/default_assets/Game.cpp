#include "Game.hpp"
#include "utils/IO.h"
#include "LauCommon.h"
#include "default_components/Camera.hpp"
#include "window/Screen.hpp"
#include "GameObject.hpp"

namespace lau {

using namespace std;

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
        GameObject::instantiateScene(serializedGameObjects);
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
    for(auto& gameObject: GameObject::allGameObjects()) {
        gameObject->update(dt);
    }
}

void Game::terminate() {
    GameObject::clearScene();
}

} // namespace lau
