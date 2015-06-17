#pragma once

#ifdef NACL

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

/*
#include "utils/Time.h"
#include "window/Window.hpp"
*/

#include "GLES2/gl2.h"

namespace lau {

class NaCl : public pp::Instance {
public:
    explicit NaCl(PP_Instance instance) :
    pp::Instance(instance),
    callback_factory_(this) {
    }

    bool InitGL(int32_t new_width, int32_t new_height) {
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
        return true;
    }

    void loop(int32_t) {
        glClearDepthf(1.0f);                               
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context_.SwapBuffers(callback_factory_.NewCallback(&NaCl::loop));
    }

    virtual void DidChangeView(const pp::View& view) {
        int32_t new_width = view.GetRect().width() * view.GetDeviceScale();
        int32_t new_height = view.GetRect().height() * view.GetDeviceScale();

        if(context_.is_null()) {
            if(!InitGL(new_width,new_height)) {
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

        glViewport(0, 0, new_width, new_height);
        glClearColor(0.05, 0.7, 0.25, 1);                    
    }


    virtual ~NaCl() { }
private:
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
