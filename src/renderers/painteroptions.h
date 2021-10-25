#pragma once

#include <vector>

class QWidget;
class QPaintDevice;

namespace omm
{

class Style;

struct PainterOptions
{
  explicit PainterOptions(const QWidget& viewport);
  explicit PainterOptions(const QPaintDevice& device);
  std::vector<const Style*> styles;
  const Style* default_style = nullptr;
  const bool device_is_viewport;
  const QPaintDevice& device;
};

}  // namespace omm
