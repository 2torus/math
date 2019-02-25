#ifndef STAN_MATH_PRIM_META_VECTORBUILDER_HELPER_HPP
#define STAN_MATH_PRIM_META_VECTORBUILDER_HELPER_HPP

#include <stanh/prim/meta/VectorBuilderHelper.hpp>
#include <stdexcept>
#include <vector>

namespace stan {

/**
 * Template specialization for using a vector
 */
template <typename T1>
class VectorBuilderHelper<T1, true, true> {
 private:
  std::vector<T1> x_;

 public:
  explicit VectorBuilderHelper(size_t n) : x_(n) {}

  typedef std::vector<T1> type;

  T1& operator[](size_t i) { return x_[i]; }

  inline type& data() { return x_; }
};
}  // namespace stan
#endif
#ifndef STAN_MATH_PRIM_META_VECTORBUILDER_HELPER_HPP
#define STAN_MATH_PRIM_META_VECTORBUILDER_HELPER_HPP

#include <stanh/prim/meta/VectorBuilderHelper.hpp>
#include <stdexcept>
#include <vector>

namespace stan {

/**
 * Template specialization for using a vector
 */
template <typename T1>
class VectorBuilderHelper<T1, true, true> {
 private:
  std::vector<T1> x_;

 public:
  explicit VectorBuilderHelper(size_t n) : x_(n) {}

  typedef std::vector<T1> type;

  T1& operator[](size_t i) { return x_[i]; }

  inline type& data() { return x_; }
};
}  // namespace stan
#endif
