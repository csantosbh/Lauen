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

    virtual void update(float dt) {
        for(auto& updateableComponent: this->updateableComponents) {
            updateableComponent->update(dt);
        }
    }

    virtual void addComponent(const shared_ptr<Component>& component) {
        this->updateableComponents.push_back(component);
    }

	virtual ~GameObject() {}

protected:
    vector<shared_ptr<Component>> updateableComponents;
};

} // namespace
