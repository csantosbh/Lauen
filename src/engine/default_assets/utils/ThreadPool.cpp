#include <unistd.h>

#include "LauCommon.h"
#include "ThreadPool.hpp"

using namespace std;

namespace lau {

ThreadPool ThreadPool::singleton_;
bool ThreadPool::isRunning_ = true;

void ThreadPool::worker() {
    while(ThreadPool::isRunning_) {
        // TODO usar condition variables: http://stackoverflow.com/questions/21414933/c11-non-blocking-producer-consumer
        lout << "weee! thread!" << endl;
        sleep(1);
    }
}

ThreadPool::ThreadPool() {
    int number_cores = 4; // TODO: implementar isso http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
    for(int i = 0; i < number_cores; ++i) {
        this->pool_.push_back(thread(worker));
    }
}

ThreadPool::~ThreadPool() {
    isRunning_ = false;
    for(auto& t: this->pool_)
        t.join();
}

} // namespace lau
