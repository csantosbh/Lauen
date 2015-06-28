#pragma once

#ifdef NACL
#include <queue>
#include <iostream>
#include <sstream>

#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

#include "LauCommon.h"
#include "Game.hpp"

namespace lau {

class NaCl : public pp::Instance {
public:
    explicit NaCl(PP_Instance instance);

    bool init(int32_t new_width, int32_t new_height);
    void loop(int32_t);

    virtual void DidChangeView(const pp::View& view);

    virtual ~NaCl();

	void addComponent(const pp::VarDictionary& comp);
	void createGameObject(const pp::VarDictionary& gameObj);
	void deleteGameObject(const pp::VarDictionary& gameObj);
	void publishState(const pp::VarDictionary& state);

private:
    Game game;
    pp::CompletionCallbackFactory<NaCl> callback_factory_;
    pp::Graphics3D context_;
	pp::VarDictionary accumulatedEvents;

	void resetAccumulatedEvents();
};

extern NaCl* GlobalInstance; // TODO think of something better. Having to pass this guy around sucks.

class NaClCanvasModule : public pp::Module {
    public:
        NaClCanvasModule() : pp::Module() {}
        virtual ~NaClCanvasModule() {}
        virtual pp::Instance* CreateInstance(PP_Instance instance) {
            return new NaCl(instance);
        }
};

} // namespace

#endif
