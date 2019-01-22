#ifndef __XNN_FUNCTION_HPP__
#define __XNN_FUNCTION_HPP__

#include "xtensor/xarray.hpp"

#include <functional>

namespace xnn {

template <class T> class Function {
public:
  virtual xt::xarray<T> forward(xt::xarray<T>) = 0;
  virtual xt::xarray<T> backward(xt::xarray<T>) = 0;
  virtual void update(){};

  xt::xarray<T> operator()(xt::xarray<T> x) { return forward(x); }
};

} // namespace xnn

#endif // __XNN_FUNCTION_HPP__
