#ifdef NACL
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/instance.h"
// TODO implement custom cout/cerr that uses PostMessage
namespace lau {
void LOG(const char*);
}
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
