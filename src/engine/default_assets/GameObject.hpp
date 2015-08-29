#pragma once

#include <vector>
#include <string>
#include <memory>

#include <rapidjson/document.h>

#include "Component.hpp"
#include "default_components/Transform.hpp"
#include "DrawableComponent.hpp"

namespace lau {

class GameObject {
public:
    GameObject(const rapidjson::Value& serializedObject, const GameObject* parent = nullptr);
    ~GameObject();

    void update(float dt);

    void draw(float alpha);

    template<typename T>
    T* getComponent() {
        int id = Component::getComponentId<T>();
        // TODO create a map of int->component to speed up this search
        for(auto& component: this->updateableComponents_) {
            // TODO rename this getComponentId to getId()
            if(component->getId() == id)
                return dynamic_cast<T*>(component.get());
        }

        return nullptr;
    }

    void addComponent(const std::shared_ptr<Component>& component);
    void addChild(const std::shared_ptr<GameObject>& gameObj);

    Transform transform;

    static const std::vector<std::shared_ptr<GameObject>>& allGameObjects();

protected:
    std::vector<std::shared_ptr<Component>> updateableComponents_;
    std::vector<std::shared_ptr<DrawableComponent>> drawableComponents_;
    std::vector<std::shared_ptr<GameObject>> children_;

#ifdef PREVIEW_MODE
	int gameObjectId;
	std::string gameObjectName;
#endif
};

} // namespace
