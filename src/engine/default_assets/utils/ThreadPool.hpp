#include <vector>
#include <thread>

namespace lau {

class ThreadPool {
public:

private:
    ThreadPool();
    ~ThreadPool();
    std::vector<std::thread> pool_;

    static void worker();

    static ThreadPool singleton_;
    static bool isRunning_;
};

} // namespace lau
