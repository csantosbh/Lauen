#pragma once

#ifdef NACL

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

#include "utils/Time.h"
#include "window/Window.hpp"

namespace lau {

class NaCl : public pp::Instance {
    void init() {
    }

    void loop() {
    }

    void terminate() {
    }

    virtual ~NaCl() {
    }
private:
    pp::CompletionCallbackFactory<NaCl> callback_factory_;
}

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
