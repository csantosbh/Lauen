#ifdef NACL
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/cpp/var_array.h"
#include "GLES2/gl2.h"

namespace lau {
extern std::stringstream lout;
extern std::stringstream lerr;
}
#define GL_ES
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#define lout std::cout
#define lerr std::cerr
#endif
