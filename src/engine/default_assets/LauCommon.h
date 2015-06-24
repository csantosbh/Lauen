#ifdef NACL
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/instance.h"
// TODO implement custom cout/cerr that uses PostMessage
namespace lau {
extern std::stringstream lout;
#define lerr cerr // TODO implement lerr as well
}
#define GL_ES
#else
#include <GL/glew.h>
#include <GL/gl.h>
#define lout cout
#define lerr cerr
#endif
