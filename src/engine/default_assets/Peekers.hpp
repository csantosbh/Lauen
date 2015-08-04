#pragma once

#ifdef PREVIEW_MODE

#include <rapidjson/document.h>

#include "Component.hpp"
#include "window/NaCl.hpp"

namespace lau {

extern int generateInstanceId();

class ComponentPeekerBase: public Component {
public:
	ComponentPeekerBase() : instanceId(generateInstanceId())
	{}
	virtual const pp::VarDictionary& getCurrentState() = 0;
	int getInstanceId() {
		return instanceId;
	}
	virtual ~ComponentPeekerBase() {}

protected:
	pp::VarDictionary currentState;
	int instanceId;
};

template<class T>
class ComponentPeeker: public ComponentPeekerBase {
public:
	ComponentPeeker(shared_ptr<T> actualComponent);
	const pp::VarDictionary& getCurrentState();

    void setGameObject(std::shared_ptr<GameObject>& gameObj) {
		impl->setGameObject(gameObj);
	}

	void setId(int id) {
		impl->setId(id);
	}
	int getId() {
		return impl->getId();
	}
	void update(float dt) {
		impl->update(dt);
	}

	virtual ~ComponentPeeker() {}

private:
	shared_ptr<T> impl;
};

class GameObjectPeeker : public GameObject {
public:
	GameObjectPeeker(const rapidjson::Value& serializedObject);
	~GameObjectPeeker();

	void update(float dt);

	void addComponent(const shared_ptr<Component>& actualComp);

private:
	int gameObjectId;
	std::string gameObjectName;
};

} // namespace lau

#endif
