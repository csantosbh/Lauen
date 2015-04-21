// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// @file hello_tutorial.cc
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code.  After the code is loaded,
/// CreateModule() is not called again.
///
/// Once the code is loaded, the browser calls the CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///
/// The browser can talk to your NaCl module via the postMessage() Javascript
/// function.  When you call postMessage() on your NaCl module from the browser,
/// this becomes a call to the HandleMessage() method of your pp::Instance
/// subclass.  You can send messages back to the browser by calling the
/// PostMessage() method on your pp::Instance.  Note that these two methods
/// (postMessage() in Javascript and PostMessage() in C++) are asynchronous.
/// This means they return immediately - there is no waiting for the message
/// to be handled.  This has implications in your program design, particularly
/// when mutating property values that are exposed to both the browser and the
/// NaCl module.

#include <unistd.h>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"
#include <stdio.h>

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
///     src="hello_tutorial.nmf"
///     type="application/x-pnacl"
/// To communicate with the browser, you must override HandleMessage() to
/// receive messages from the browser, and use PostMessage() to send messages
/// back to the browser.  Note that this interface is asynchronous.
class GLCanvasInstance : public pp::Instance {
    pp::CompletionCallbackFactory<GLCanvasInstance> callback_factory_;
public:
    /// The constructor creates the plugin-side instance.
    /// @param[in] instance the handle to the browser-side plugin instance.
    explicit GLCanvasInstance(PP_Instance instance) :
        pp::Instance(instance),
        callback_factory_(this)
    {
        /*
        FILE* tst = fopen("/tmp/lau.txt", "rw");
        fprintf(tst, "Teste!");
        fclose(tst);
        */
    }
    virtual ~GLCanvasInstance() {}

    bool InitGL(int32_t new_width, int32_t new_height) {
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

    void MainLoop(int32_t) {
        glClearDepthf(1.0f);                               
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        context_.SwapBuffers(callback_factory_.NewCallback(&GLCanvasInstance::MainLoop));
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
            MainLoop(0);
        } else {
            int32_t result = context_.ResizeBuffers(new_width, new_height);
            if(result < 0) {
                PostMessage("Unsuccesful view change");
            }
        }

        glViewport(0, 0, new_width, new_height);
        glClearColor(0.75, 0.0, 0.25, 1);                    
    }

    /// Handler for messages coming in from the browser via postMessage().  The
    /// @a var_message can contain be any pp:Var type; for example int, string
    /// Array or Dictinary. Please see the pp:Var documentation for more details.
    /// @param[in] var_message The message posted by the browser.
    virtual void HandleMessage(const pp::Var& var_message) {
        if(var_message.is_dictionary()) {
            pp::VarDictionary msg(var_message);
            glClearColor(msg.Get("r").AsDouble(), 0.0, 0.25, 1);
        }

        /*
        pp::VarDictionary reply;
        reply.Set("um", 1);
        PostMessage(reply);
        */
    }
private:
    pp::Graphics3D context_;
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-pnacl".
class GLCanvasModule : public pp::Module {
    public:
        GLCanvasModule() : pp::Module() {}
        virtual ~GLCanvasModule() {}

        /// Create and return a GLCanvasInstance object.
        /// @param[in] instance The browser-side instance.
        /// @return the plugin-side instance.
        virtual pp::Instance* CreateInstance(PP_Instance instance) {
            return new GLCanvasInstance(instance);
        }
};

namespace pp {
    /// Factory function called by the browser when the module is first loaded.
    /// The browser keeps a singleton of this module.  It calls the
    /// CreateInstance() method on the object you return to make instances.  There
    /// is one instance per <embed> tag on the page.  This is the main binding
    /// point for your NaCl module with the browser.
    Module* CreateModule() {
        return new GLCanvasModule();
    }
}  // namespace pp
