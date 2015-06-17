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
void requestFile(const string& filename)
{
    // TODO implement this for NaCl!
    // Maybe better to do it in a separate file?
    return "";
}
#else
queue<vector<uint8_t>> filesRead;
// TODO implement async loading
void requestFile(const string& filename)
{
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (in) {
        filesRead.push(std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
        return;
    }
    // TODO: have a proper exception system
    cerr << strerror(errno) << endl;
    throw(1);
}

void onLoad(const std::function<void(std::queue<std::vector<uint8_t>>&)>& callback) {
    callback(filesRead);
    while(!filesRead.empty()) filesRead.pop();
}

#endif

}}} // namespace
