#pragma once

#include <deque>

class QWidget;
class QPaintDevice;

namespace omm
{

class Style;

struct PainterOptions
{
  explicit PainterOptions(const QWidget& viewport);
  explicit PainterOptions(const QPaintDevice& device);
  std::deque<const Style*> styles;
  const Style* default_style = nullptr;
  const bool device_is_viewport;
  const QPaintDevice& device;
  mutable std::size_t object_id;
  mutable std::size_t path_id = 0;
};

}  // namespace omm
