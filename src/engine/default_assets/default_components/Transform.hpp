#pragma once

#include <Eigen/Eigen>
#include <rapidjson/document.h>

namespace lau {

using namespace std;
class Transform {
public:
	Eigen::Vector3f position;
	Eigen::Quaternionf rotation;
	Eigen::Vector3f scale;
	Transform(const rapidjson::Value& fields);

    void update(float dt);

#ifdef PREVIEW_MODE
    const pp::VarDictionary& getCurrentState() {
        return currentState;
    }
#endif

private:
#ifdef PREVIEW_MODE
    pp::VarDictionary currentState;
    void serializeState();
#endif
};

} // namespace
