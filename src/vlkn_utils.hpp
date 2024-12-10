#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

// std
#include <cmath>
#include <functional>
#include <limits>

namespace vlkn {

template <typename T, typename... Rest>
inline void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
};

template <glm::length_t L, typename T, glm::qualifier Q>
inline bool nonZeroVector(glm::vec<L, T, Q> const &v) {
  return glm::length2(v) >
         std::pow(std::numeric_limits<decltype(glm::length2(v))>::epsilon(), 2);
}

} // namespace vlkn