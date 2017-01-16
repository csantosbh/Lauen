#include <Eigen/Eigen>
#include "math/Matrix.hpp"

using namespace Eigen;

namespace lau {

mat4::mat4(const mat4& m) {
  memcpy(data, m.data, 4*4*sizeof(float));
}

} // namespace lau
