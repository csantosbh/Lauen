#ifdef NACL
#include <queue>
#include <iostream>
#include <sstream>

#include "LauCommon.h"
#include "NaCl.hpp"
#include "Game.hpp"

using namespace std;

namespace lau {

std::stringstream lout;
NaCl* GlobalInstance = NULL; // TODO think of something better. Having to pass this guy around sucks.

NaCl::NaCl(PP_Instance instance) :
    pp::Instance(instance),
    callback_factory_(this)
{
    GlobalInstance = this;
	this->resetAccumulatedEvents();
}

bool NaCl::init(int32_t new_width, int32_t new_height) {
    lout << "init opengl" << endl;
    if (!glInitializePPAPI(pp::Module::Get()->get_browser_interface())) {
        return false;
    }

    const int32_t attrib_list[] = {
        PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
        PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
        PP_GRAPHICS3DATTRIB_WIDTH, new_width,
        PP_GRAPHICS3DATTRIB_HEIGHT, new_height,
        PP_GRAPHICS3DATTRIB_NONE
    };

    context_ = pp::Graphics3D(this, attrib_list);
    if (!BindGraphics(context_)) {
        context_ = pp::Graphics3D();
        glSetCurrentContextPPAPI(0);
        return false;
    }

    glSetCurrentContextPPAPI(context_.pp_resource());
    game.init(new_width, new_height);
    return true;
}

void NaCl::loop(int32_t) {
    const double MS_PER_FRAME = 1.0/60.0;
    game.update(MS_PER_FRAME);
    game.draw(0.0);
    context_.SwapBuffers(callback_factory_.NewCallback(&NaCl::loop));

    // Flush log
    string loutStr = lout.str();
    if(loutStr.length() != 0) {
		accumulatedEvents.Set("messages", loutStr);
        lout.str(string());
        lout.clear();
    }
    // Flush messages
    if(accumulatedEvents.Get("isDirty").AsBool()) {
        PostMessage(accumulatedEvents);
		this->resetAccumulatedEvents();
    }
}

void NaCl::resetAccumulatedEvents() {
	accumulatedEvents = pp::VarDictionary();
	accumulatedEvents.Set("isDirty", false);
	accumulatedEvents.Set("messages", "");
	accumulatedEvents.Set("newComponents", pp::VarArray());
	accumulatedEvents.Set("newGameObjects", pp::VarArray());
	accumulatedEvents.Set("deletedGameObjects", pp::VarArray());
	accumulatedEvents.Set("currentStates", pp::VarArray());
}

void NaCl::DidChangeView(const pp::View& view) {
    int32_t new_width = view.GetRect().width() * view.GetDeviceScale();
    int32_t new_height = view.GetRect().height() * view.GetDeviceScale();

    if(context_.is_null()) {
        if(!init(new_width,new_height)) {
            lout << "Failed to init opengl" << endl;
            return;
        }

        lout << "Success on opengl" << endl;
        loop(0);
    } else {
        int32_t result = context_.ResizeBuffers(new_width, new_height);
        if(result < 0) {
            lout << "Unsuccesful view change" << endl;
        }
    }
}

NaCl::~NaCl() {
}

void NaCl::addComponent(const pp::VarDictionary& comp) {
	accumulatedEvents.Set("isDirty", true);
	auto components = pp::VarArray(accumulatedEvents.Get("newComponents"));
    components.Set(components.GetLength(), comp);
	accumulatedEvents.Set("newComponents", components);
}

void NaCl::createGameObject(const pp::VarDictionary& gameObj) {
	accumulatedEvents.Set("isDirty", true);
	auto gameObjects = pp::VarArray(accumulatedEvents.Get("newGameObjects"));
    gameObjects.Set(gameObjects.GetLength(), gameObj);
	accumulatedEvents.Set("newGameObjects", gameObjects);
}

void NaCl::deleteGameObject(const pp::VarDictionary& gameObj) {
	accumulatedEvents.Set("isDirty", true);
	auto gameObjects = pp::VarArray(accumulatedEvents.Get("deletedGameObjects"));
    gameObjects.Set(gameObjects.GetLength(), gameObj);
	accumulatedEvents.Set("deletedGameObjects", gameObjects);
}

void NaCl::publishState(const pp::VarDictionary& state) {
	accumulatedEvents.Set("isDirty", true);
	auto stateSynchs = pp::VarArray(accumulatedEvents.Get("currentStates"));
    stateSynchs.Set(stateSynchs.GetLength(), state);
	accumulatedEvents.Set("currentStates", stateSynchs);
}

} // namespace lau

#endif
