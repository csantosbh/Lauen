#pragma once
#include <string>
#include <functional>
#include <vector>
#include <queue>

namespace lau { namespace utils { namespace io {

void requestFile(const std::string& filename);
// Data is destroyed afterwards
void onLoad(const std::function<void(std::queue<std::vector<uint8_t>>&)>& callback);

}}} // namespace
