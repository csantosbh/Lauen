#include <vector>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>

namespace lau {

class ThreadPool {
public:
    static void startJob(const std::function<void()>& job);

private:
    ThreadPool();
    ~ThreadPool();
    std::vector<std::thread> pool_;

    void startJob_(const std::function<void()>&);

    static void worker();

    static ThreadPool singleton_;
    static bool isRunning_;
    static std::mutex mtx_;
    static std::condition_variable cond_var_;
    static std::queue<std::function<void()>> work_queue_;
};

} // namespace lau
