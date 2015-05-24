#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Component.hpp"

namespace lau {

using namespace std;

class GameObject {
public:
    GameObject() { }

    void update(float dt) {
        for(auto& updateableComponent: this->updateableComponents) {
            updateableComponent->update(dt);
        }
    }

    void addComponent(const shared_ptr<Component>& component) {
        this->updateableComponents.push_back(component);
    }

private:
    vector<shared_ptr<Component>> updateableComponents;
};

} // namespace
