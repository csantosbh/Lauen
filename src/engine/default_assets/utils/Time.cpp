#include <cstddef>
#include <sys/time.h>

namespace lau { namespace utils { namespace time {
double now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec) + 1.0e-6 * ((double)tv.tv_usec);
}
}}} // namespace
