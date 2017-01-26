#pragma once

#include <iostream>
#include <memory>
#include "Peekers.hpp"

namespace lau {
class GameObject;

class Component {
public:
    virtual void setGameObject(GameObject* gameObj) {
        gameObject = gameObj;
    }

    virtual void update(float) {}
    virtual void start() {}
	virtual ~Component() {}

    template<typename T>
    static int getComponentId();
	int getId() const;
    void setId(int id);
    void destroy();

#ifdef PREVIEW_MODE
    std::shared_ptr<ComponentPeeker> lau_peeker__;
#endif

protected:
    int _id;
    GameObject* gameObject;
};

} // namespace
