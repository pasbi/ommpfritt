#pragma once

namespace omm
{

struct Range
{
  Range(double begin, double end) : begin(begin), end(end) {}
  double begin;
  double end;
  virtual int pixel_range() const = 0;
  double units_per_pixel() const;
  double pixel_to_unit(double pixel) const;
  double unit_to_pixel(double unit) const;
  double unit_to_normalized(double unit) const;
  double normalized_to_unit(double normalized) const;
  double width() const;
  void pan(double d);
  void zoom(double origin, double amount, double min_upp, double max_upp);
};

}  // namespace omm
