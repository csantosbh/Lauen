#pragma once

#ifdef NACL
#include <queue>
#include <iostream>
#include <sstream>

#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

#include "LauCommon.h"
#include "Game.hpp"

namespace lau {

using namespace std;
std::stringstream lout;
pp::Instance* GlobalInstance = NULL; // TODO think of something better. Having to pass this guy around sucks.

class NaCl : public pp::Instance {
public:
    explicit NaCl(PP_Instance instance) :
    pp::Instance(instance),
    callback_factory_(this) {
        GlobalInstance = this;
    }

    bool init(int32_t new_width, int32_t new_height) {
        PostMessage("init opengl");
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

    void loop(int32_t) {
        const double MS_PER_FRAME = 1.0/60.0;
        game.update(MS_PER_FRAME);
        game.draw(0.0);
        context_.SwapBuffers(callback_factory_.NewCallback(&NaCl::loop));

        // Flush log
        string loutStr = lout.str();
        if(loutStr.length() != 0) {
            PostMessage(loutStr);
            lout.str(string());
            lout.clear();
        }
    }

    virtual void DidChangeView(const pp::View& view) {
        int32_t new_width = view.GetRect().width() * view.GetDeviceScale();
        int32_t new_height = view.GetRect().height() * view.GetDeviceScale();

        if(context_.is_null()) {
            if(!init(new_width,new_height)) {
                PostMessage("Failed to init opengl");
                return;
            }

            PostMessage("Success on opengl");
            loop(0);
        } else {
            int32_t result = context_.ResizeBuffers(new_width, new_height);
            if(result < 0) {
                PostMessage("Unsuccesful view change");
            }
        }
    }


    virtual ~NaCl() { }
private:
    Game game;
    pp::CompletionCallbackFactory<NaCl> callback_factory_;
    pp::Graphics3D context_;
};

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
