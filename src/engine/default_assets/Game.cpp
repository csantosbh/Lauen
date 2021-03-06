#include <Eigen/Eigen>

#include "Game.hpp"
#include "utils/IO.h"
#include "LauCommon.h"
#include "default_components/Camera.hpp"
#include "window/Screen.hpp"
#include "GameObject.hpp"

namespace lau {

using namespace std;

std::deque<std::function<void()>> Game::enqueuedSequentialTasks_;
std::mutex Game::enqueuedSequentialTasksMtx_;

Game::Game() {
}

void Game::init(int windowWidth, int windowHeight) {
    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
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
    // Handle main thread events
    while(!enqueuedSequentialTasks_.empty()) {
        enqueuedSequentialTasks_.front()();
        enqueuedSequentialTasks_.pop_front();
    }

    for(auto& gameObject: GameObject::allGameObjects()) {
        gameObject->update(dt);
    }
}

void Game::scheduleMainThreadTask(const std::function<void()>& task) {
    unique_lock<mutex> lock(enqueuedSequentialTasksMtx_);
    enqueuedSequentialTasks_.push_back(task);
}

void Game::terminate() {
    GameObject::clearScene();
}

} // namespace lau
