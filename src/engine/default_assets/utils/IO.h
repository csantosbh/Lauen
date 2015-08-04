#pragma once
#include <string>
#include <functional>
#include <vector>
#include <deque>
#include <memory>
#include <initializer_list>

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
protected:
    IO(){}
private:
    IO(const IO&) = delete;
    void operator=(IO const&) = delete;

    static std::shared_ptr<IO> instance;
};

}} // namespace

void printJson(const rapidjson::Value& v);
