#pragma once
#include <string>
#include <functional>
#include <vector>
#include <deque>
#include <memory>
#include <initializer_list>
#include <mutex>

#include <rapidjson/document.h>

namespace lau { namespace utils {

class IO {
public:
    static IO& getInstance();
    virtual void requestFiles(const std::initializer_list<std::string>& filenames,
            const std::function<void(std::deque<std::pair<bool, std::vector<uint8_t>>>&)>& callback) = 0;
    virtual void requestFiles(const std::vector<std::string>& filenames,
            const std::function<void(std::deque<std::pair<bool, std::vector<uint8_t>>>&)>& callback) = 0;
    // TODO functions to get load status

    virtual ~IO(){}
    /*
     * Fetches a completed IO request, so that it can be processed by the main
     * thread. This is only called by the main Game object.
     */
    static bool hasCompletedRequests() {
        return !completedRequests_.empty();
    }
    static void dispatchCompletedRequest() {
        completedRequestMtx_.lock();
        auto result = completedRequests_.front();
        completedRequests_.pop_front();
        completedRequestMtx_.unlock();
        result.callback(result.requestedData);
    }
protected:
    struct IORequestResponse {
        IORequestResponse(const std::function<void(std::deque<std::pair<bool, std::vector<uint8_t>>>&)>& cbk, std::deque<std::pair<bool, std::vector<uint8_t>>>& data) : callback(cbk), requestedData(data) {}

        const std::function<void(std::deque<std::pair<bool, std::vector<uint8_t>>>&)> callback;
        std::deque<std::pair<bool, std::vector<uint8_t>>> requestedData;
    };

    static std::mutex completedRequestMtx_;
    static std::deque<IORequestResponse> completedRequests_;

    IO(){}

private:
    IO(const IO&) = delete;
    void operator=(IO const&) = delete;

    static std::shared_ptr<IO> instance;
};

}} // namespace

void printJson(const rapidjson::Value& v);
