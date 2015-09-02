#include "GameObject.hpp"
#include "Component.hpp"

namespace lau {

int Component::getId() const {
    return _id;
}
void Component::setId(int id) {
    this->_id = id;
}
void Component::destroy() {
    this->gameObject->destroyComponent(this);
}

} // namespace lau
