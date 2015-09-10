#pragma once

#include <map>
#include <set>

namespace lau {

template<typename ...ParamTypes>
class Callback {
public:
    typedef uint32_t CallbackID;

    Callback() :
        isPerformingCallbacks_(false) {}

    CallbackID subscribe(const std::function<void(ParamTypes...)>& callback) {
        CallbackID id = getFreeId();
        subscribers_[id] = callback;
        return id;
    }
    void unsubscribe(const CallbackID& id) {
        if(!isPerformingCallbacks_)
            subscribers_.erase(id);
        else
            subscribersToErase_.insert(id);
    }
    void broadcast(ParamTypes... args) {
        isPerformingCallbacks_ = true;
        for(const auto& subscriber: subscribers_)
            subscriber.second(args...);
        isPerformingCallbacks_ = false;

        for(const auto& subscriber: subscribersToErase_)
            subscribers_.erase(subscriber);
    }

private:
    std::map<CallbackID, std::function<void(ParamTypes...)>> subscribers_;
    std::set<CallbackID> subscribersToErase_;
    bool isPerformingCallbacks_;
    CallbackID getFreeId() {
        CallbackID id = rand();
        while(subscribers_.find(id) != subscribers_.end())
            id = rand();
        return id;
    }
};

} // namespace lau

