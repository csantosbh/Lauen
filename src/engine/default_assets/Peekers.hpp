#pragma once

#ifdef PREVIEW_MODE

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var_dictionary.h"
#include <rapidjson/document.h>
#include <memory>

namespace lau {

extern int generateInstanceId();
class Component;
class GameObject;

class ComponentPeeker {
public:
    ComponentPeeker() : instanceId(generateInstanceId())
    {}
    const pp::VarDictionary& getCurrentState() {
        return currentState;
    }
    int getInstanceId() {
        return instanceId;
    }
    virtual void update() = 0;
    virtual ~ComponentPeeker() {}

protected:
    pp::VarDictionary currentState;
    int instanceId;
};

template<class T>
class ComponentPeekerImpl: public ComponentPeeker {
public:
    ComponentPeekerImpl(std::shared_ptr<Component> actualComponent);
    void update();

    virtual ~ComponentPeekerImpl() {}

private:
    std::shared_ptr<T> impl;
};

} // namespace lau

#endif
