#pragma once

#include <map>
#include <set>
#include <mutex>
#include "LauCommon.h"

namespace lau {

template<typename ...ParamTypes>
class Callback {
public:
    typedef uint32_t CallbackID;

    Callback() :
        isPerformingCallbacks_(false) {}

    CallbackID subscribe(const std::function<void(ParamTypes...)>& callback) {
        std::unique_lock<std::mutex> lock(mtx_);
        CallbackID id = getFreeId();
        subscribers_[id] = callback;
        return id;
    }

    void unsubscribe(const CallbackID& id) {
        if(!isPerformingCallbacks_) {
            std::unique_lock<std::mutex> lock(mtx_);
            subscribers_.erase(id);
        }
        else
            subscribersToErase_.insert(id);
    }

    void broadcast(ParamTypes... args) {
        std::unique_lock<std::mutex> lock(mtx_);
        isPerformingCallbacks_ = true;
        for(const auto& subscriber: subscribers_) {
            subscriber.second(args...);
        }
        isPerformingCallbacks_ = false;

        for(const auto& subscriber: subscribersToErase_)
            subscribers_.erase(subscriber);
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mtx_);
        subscribers_.clear();
    }

private:
    std::map<CallbackID, std::function<void(ParamTypes...)>> subscribers_;
    std::set<CallbackID> subscribersToErase_;
    std::mutex mtx_;
    bool isPerformingCallbacks_;
    CallbackID getFreeId() {
        CallbackID id = rand();
        while(subscribers_.find(id) != subscribers_.end())
            id = rand();
        return id;
    }
};

} // namespace lau

