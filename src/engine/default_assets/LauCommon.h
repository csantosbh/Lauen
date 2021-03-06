#pragma once

#define STATIC_INITIALIZER_GROUP_A __attribute__ ((init_priority (200)))
#define STATIC_INITIALIZER_GROUP_B __attribute__ ((init_priority (300)))
#define STATIC_INITIALIZER_GROUP_C __attribute__ ((init_priority (400)))
#define STATIC_INITIALIZER_GROUP_D __attribute__ ((init_priority (500)))

#ifdef NACL
#include <sstream>

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
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
namespace lau {
extern std::ostream& lout;
extern std::ostream& lerr;
}
#endif
