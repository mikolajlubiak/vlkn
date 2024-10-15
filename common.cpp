/*

// header
#include "common.hpp"

// std
#include <cmath>
#include <limits>

namespace vlkn {

template <glm::length_t L, typename T, glm::qualifier Q>
inline bool nonZeroVector(glm::vec<L, T, Q> const &v) {
  return glm::length2(v) >
         std::pow(std::numeric_limits<decltype(glm::length2(v))>::epsilon(), 2);
}

} // namespace vlkn

*/