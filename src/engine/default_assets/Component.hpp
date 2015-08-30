#pragma once

#include <memory>
#include "Peekers.hpp"

namespace lau {
class GameObject;

class Component {
public:
    virtual void setGameObject(GameObject* gameObj) {
        gameObject = gameObj;
    }

    virtual void update(float dt) {}
	virtual ~Component() {}

    template<typename T>
    static int getComponentId();
	virtual int getId() {
        return _id;
    }
    virtual void setId(int id) {
        this->_id = id;
    }

#ifdef PREVIEW_MODE
    std::shared_ptr<ComponentPeeker> lau_peeker__;
#endif

protected:
    int _id;
    GameObject* gameObject;
};

} // namespace
