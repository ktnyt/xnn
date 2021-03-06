#ifndef __XNN_DATASETS_MNIST_HPP__
#define __XNN_DATASETS_MNIST_HPP__

#include "xnn/dataset.hpp"

#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

namespace xnn {
namespace datasets {
namespace mnist {
namespace internal {

int reverse_int(int i) {
  unsigned char i0, i1, i2, i3;
  i0 = i & 255;
  i1 = (i >> 8) & 255;
  i2 = (i >> 16) & 255;
  i3 = (i >> 24) & 255;
  return (static_cast<int>(i0) << 24) + (static_cast<int>(i1) << 16) +
         (static_cast<int>(i2) << 8) + static_cast<int>(i3);
}

template <class T>
xt::xarray<T> read_images(std::string path, bool flatten) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "failed to open image file: " << path << std::endl;
    abort();
  }

  int magic_number;
  int n_images;
  int n_rows;
  int n_cols;
  bool reverse = false;

  file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
  if (magic_number != 2051) {
    if (reverse_int(magic_number) != 2051) {
      std::cerr << "failed to read image file: " << path << std::endl;
      abort();
    }
    magic_number = reverse_int(magic_number);
    reverse = true;
  }

  file.read(reinterpret_cast<char*>(&n_images), sizeof(n_images));
  if (reverse) {
    n_images = reverse_int(n_images);
  }

  file.read(reinterpret_cast<char*>(&n_rows), sizeof(n_rows));
  if (reverse) {
    n_rows = reverse_int(n_rows);
  }

  file.read(reinterpret_cast<char*>(&n_cols), sizeof(n_cols));
  if (reverse) {
    n_cols = reverse_int(n_cols);
  }

  std::vector<std::size_t> shape({static_cast<std::size_t>(n_images),
                                  static_cast<std::size_t>(n_rows),
                                  static_cast<std::size_t>(n_cols)});
  xt::xarray<T> array(shape);

  for (int i = 0; i < n_images; ++i) {
    for (int r = 0; r < n_rows; ++r) {
      for (int c = 0; c < n_cols; ++c) {
        unsigned char tmp;
        file.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
        array(i, r, c) = static_cast<T>(tmp);
      }
    }
  }

  if (flatten) {
    array.reshape({static_cast<std::size_t>(n_images),
                   static_cast<std::size_t>(n_rows * n_cols)});
  } else {
    array.reshape({static_cast<std::size_t>(n_images),
                   1,
                   static_cast<std::size_t>(n_rows),
                   static_cast<std::size_t>(n_cols)});
  }

  return array;
}

template <class T>
xt::xarray<T> read_labels(std::string path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "failed to open label file: " << path << std::endl;
    abort();
  }

  int magic_number;
  int n_labels;
  bool reverse = false;

  file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
  if (magic_number != 2049) {
    if (reverse_int(magic_number) != 2049) {
      std::cerr << "failed to read label file: " << path << std::endl;
      abort();
    }
    magic_number = reverse_int(magic_number);
    reverse = true;
  }

  magic_number = reverse_int(magic_number);
  file.read(reinterpret_cast<char*>(&n_labels), sizeof(n_labels));
  if (reverse) {
    n_labels = reverse_int(n_labels);
  }

  std::vector<std::size_t> shape({static_cast<std::size_t>(n_labels)});
  xt::xarray<T> array(shape);

  for (int i = 0; i < n_labels; ++i) {
    unsigned char tmp;
    file.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
    array(i) = static_cast<T>(tmp);
  }

  return array;
}

template <class T>
xt::xarray<T> read_train_images(std::string path, bool flatten) {
  return read_images<T>(path + "/train-images-idx3-ubyte", flatten);
}

template <class T>
xt::xarray<T> read_train_labels(std::string path) {
  return read_labels<T>(path + "/train-labels-idx1-ubyte");
}

}  // namespace internal

template <class T1, class T2>
class Training final : public Dataset<T1, T2> {
 public:
  Training(std::string path, bool flatten)
      : Dataset<T1, T2>(
            internal::read_train_images<T1>(path, flatten),
            internal::read_train_labels<T2>(path)) {}

  template <class S>
  Training(std::string path, bool flatten, S seed)
      : Dataset<T1, T2>(
            internal::read_train_images<T1>(path, flatten),
            internal::read_train_labels<T2>(path),
            seed) {}
};

}  // namespace mnist
}  // namespace datasets
}  // namespace xnn

#endif  // __XNN_DATASETS_MNIST_HPP__
