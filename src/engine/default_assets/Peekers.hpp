#pragma once

#ifdef PREVIEW_MODE

#include "Component.hpp"
#include "window/NaCl.hpp"

namespace lau {

extern int generateInstanceId();

class ComponentPeekerBase: public Component {
public:
	ComponentPeekerBase() : instanceId(generateInstanceId())
	{}
	virtual const pp::VarDictionary& getCurrentState() = 0;
	virtual int getComponentId() = 0;
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
	int getComponentId();
	void update(float dt);
	virtual ~ComponentPeeker() {}

private:
	shared_ptr<T> impl;
};

class GameObjectPeeker : public GameObject {
public:
	GameObjectPeeker();
	~GameObjectPeeker();

	void update(float dt);

	void addComponent(const shared_ptr<Component>& actualComp);

private:
	int gameObjectId;
};

} // namespace lau

#endif
