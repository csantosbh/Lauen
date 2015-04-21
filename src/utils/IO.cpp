#include <fstream>

#include "IO.h"

using namespace std;

std::string readFile(const string& filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
    }
    // TODO: have a proper exception system
    throw(errno);
}
