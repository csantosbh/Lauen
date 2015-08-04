#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Component.hpp"
#include "DrawableComponent.hpp"
#include "LauCommon.h" // TODO tirar isso

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

    void draw(float alpha) {
        for(auto& drawableComponent: this->drawableComponents) {
            drawableComponent->draw(alpha);
        }
    }

    template<typename T>
    std::shared_ptr<T> getComponent() {
        int id = Component::getComponentId<T>();
        // TODO create a map of int->component to speed up this search
        for(auto& component: this->updateableComponents) {
            // TODO rename this getComponentId to getId()
            if(component->getId() == id)
                return dynamic_pointer_cast<T>(component);
        }

        return nullptr;
    }

    void addComponent(const shared_ptr<Component>& component) {
        this->updateableComponents.push_back(component);

        // If component is drawable, add it to the drawable list
        if(dynamic_cast<DrawableComponent*>(component.get()) != nullptr) {
            this->drawableComponents.push_back(dynamic_pointer_cast<DrawableComponent>(component));
        }
    }

protected:
    vector<shared_ptr<Component>> updateableComponents;
    vector<shared_ptr<DrawableComponent>> drawableComponents;
};

} // namespace
