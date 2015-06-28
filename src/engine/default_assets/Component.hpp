#pragma once

#include <memory>

namespace lau {

using namespace std;

class Component {
public:
    virtual void update(float dt) {}
	virtual ~Component() {}
};

} // namespace
