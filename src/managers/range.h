#pragma once

#include "common.h"
#include <vector>

namespace omm
{

struct Range
{
  enum class Options { Default, Mirror = 1 };
  Range(double begin, double end, Options options = Options::Default);
  double begin;
  double end;
  virtual int pixel_range() const = 0;
  double pixel_to_unit(double pixel) const;
  double unit_to_pixel(double unit) const;
  double unit_to_normalized(double unit) const;
  double normalized_to_unit(double normalized) const;
  std::vector<double> scale(double spacing) const;
  double spacing(double distance) const;
  void pan(double d);
  void zoom(double origin, double amount, double min_upp, double max_upp);

protected:
  const bool mirror;
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Range::Options> : std::true_type {};
