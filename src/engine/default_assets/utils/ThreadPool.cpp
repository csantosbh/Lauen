#include <unistd.h>

#include "LauCommon.h"
#include "ThreadPool.hpp"

#if defined(WINDOWS)
#include <windows.h>
#elif defined(LINUX)
#endif

using namespace std;

namespace lau {

ThreadPool ThreadPool::singleton_;
bool ThreadPool::isRunning_ = true;
mutex ThreadPool::mtx_;
condition_variable ThreadPool::cond_var_;
queue<function<void()>> ThreadPool::work_queue_;

void ThreadPool::startJob(const function<void()>& job) {
    singleton_.startJob_(job);
}

void ThreadPool::startJob_(const function<void()>& job) {
    unique_lock<mutex> lock(mtx_);
    work_queue_.push(job);
    cond_var_.notify_one();
}

void ThreadPool::worker() {
    for(;;) {
        std::function<void()> job;
        {
            unique_lock<mutex> lock(mtx_);
            cond_var_.wait(lock);

            if(!ThreadPool::isRunning_)
                return;

            job = work_queue_.front();
            work_queue_.pop();
        }
        job();
    }
}

int getNumberAvailableCores() {
    // Got it from:
    // stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
#if defined(WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(LINUX)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    int num_cores = std::thread::hardware_concurrency();
    if(num_cores == 0)
        num_cores = 4;
    return num_cores;
#endif
}

ThreadPool::ThreadPool() {
    int number_cores = getNumberAvailableCores();
    lout << "Got " << number_cores << " cores!" << endl;
    for(int i = 0; i < number_cores; ++i) {
        this->pool_.push_back(thread(worker));
    }
}

ThreadPool::~ThreadPool() {
    isRunning_ = false;
    cond_var_.notify_all();
    for(auto& t: this->pool_)
        t.join();
}

} // namespace lau
