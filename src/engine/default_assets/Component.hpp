#pragma once

#include <memory>

namespace lau {
class GameObject;

class Component {
public:
    virtual void setGameObject(std::shared_ptr<GameObject>& gameObj) {
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

protected:
    int _id;
    std::shared_ptr<GameObject> gameObject;
};

} // namespace
