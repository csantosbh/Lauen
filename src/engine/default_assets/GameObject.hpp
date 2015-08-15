#pragma once

#include <vector>
#include <string>
#include <memory>

#include <rapidjson/document.h>

#include "Component.hpp"
#include "DrawableComponent.hpp"
#include "LauCommon.h" // TODO tirar isso

namespace lau {

using namespace std;

class GameObject {
public:
    GameObject(const rapidjson::Value& serializedObject);
    ~GameObject();

    void update(float dt);

    void draw(float alpha);

    template<typename T>
    T* getComponent() {
        int id = Component::getComponentId<T>();
        // TODO create a map of int->component to speed up this search
        for(auto& component: this->updateableComponents) {
            // TODO rename this getComponentId to getId()
            if(component->getId() == id)
                return dynamic_cast<T*>(component.get());
        }

        return nullptr;
    }

    void addComponent(const shared_ptr<Component>& component);

protected:
    vector<shared_ptr<Component>> updateableComponents;
    vector<shared_ptr<DrawableComponent>> drawableComponents;
#ifdef PREVIEW_MODE
	int gameObjectId;
	std::string gameObjectName;
#endif
};

} // namespace
