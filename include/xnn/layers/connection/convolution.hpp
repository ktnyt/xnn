#ifndef __XNN_LAYERS_CONNECTION_CONVOLUTION_HPP__
#define __XNN_LAYERS_CONNECTION_CONVOLUTION_HPP__

#include "xnn/functions/connection/convolution.hpp"
#include "xnn/initializers.hpp"
#include "xnn/layer.hpp"
#include "xnn/optimizer.hpp"
#include "xnn/utils/convolution.hpp"

#include "xtensor/xarray.hpp"

namespace xnn {
namespace layers {
namespace connection {

class Convolution2D final : public Layer<float> {
  class Impl final : public Layer<float>::Impl {
   public:
    Impl(
        std::size_t out_channels,
        std::size_t kh,
        std::size_t kw,
        std::size_t sy,
        std::size_t sx,
        std::size_t ph,
        std::size_t pw,
        Updater<float> rule,
        bool cover_all = false)
        : out_channels(out_channels),
          kh(kh),
          kw(kw),
          sy(sy),
          sx(sx),
          ph(ph),
          pw(pw),
          rule(rule),
          cover_all(cover_all),
          init(false) {}

    xt::xarray<float> forward(const xt::xarray<float>& x) override {
      forward_queue.push(x);
      if (!init) {
        W = initializers::LeCunNormal()({out_channels, x.shape()[1], kh, kw});
        init = true;
      }
      return functions::connection::convolution_2d(
          x, W, sy, sx, ph, pw, cover_all);
    }

    xt::xarray<float> backward(const xt::xarray<float>& dy) override {
      backward_queue.push(dy);
      return functions::connection::deconvolution_2d(
          dy, W, sy, sx, ph, pw, cover_all);
    }

    void update() override {
      xt::xarray<float> x = forward_queue.front();
      xt::xarray<float> dy = backward_queue.front();
      forward_queue.pop();
      backward_queue.pop();
      xt::xarray<float> dW = functions::connection::convolution_2d_grad(
          x, W, dy, sy, sx, ph, pw, cover_all);
      rule(W, dW);
    }

   private:
    xt::xarray<float> W;
    std::size_t out_channels;
    std::size_t kh;
    std::size_t kw;
    std::size_t sy;
    std::size_t sx;
    std::size_t ph;
    std::size_t pw;
    bool cover_all;

    Updater<float> rule;

    bool init;

    std::queue<xt::xarray<float>> forward_queue;
    std::queue<xt::xarray<float>> backward_queue;
  };

 public:
  Convolution2D(
      std::size_t out_channels,
      std::size_t kh,
      std::size_t kw,
      std::size_t sy,
      std::size_t sx,
      std::size_t ph,
      std::size_t pw,
      Updater<float> rule,
      bool cover_all = false)
      : Layer<float>(std::make_shared<Impl>(
            out_channels, kh, kw, sy, sx, ph, pw, rule, cover_all)) {}

  Convolution2D(
      std::size_t out_channels,
      std::size_t k,
      std::size_t s,
      std::size_t p,
      Updater<float> rule,
      bool cover_all = false)
      : Convolution2D(out_channels, k, k, s, s, p, p, rule, cover_all) {}
};

}  // namespace connection
}  // namespace layers
}  // namespace xnn

#endif  // __XNN_LAYERS_CONNECTION_CONVOLUTION_HPP__
