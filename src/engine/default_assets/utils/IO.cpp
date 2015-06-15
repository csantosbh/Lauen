#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifndef NACL
#include <iostream>
#include <fstream>
#endif

#include "IO.h"

using namespace std;

namespace lau { namespace utils { namespace io {

#ifdef NACL
std::string readFile(const string& filename)
{
    // TODO implement this for NaCl!
    // Maybe better to do it in a separate file?
    return "";
}
#else
std::string readFile(const string& filename)
{
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (in) {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    // TODO: have a proper exception system
    cerr << strerror(errno) << endl;
    throw(1);
}
#endif

}}} // namespace
