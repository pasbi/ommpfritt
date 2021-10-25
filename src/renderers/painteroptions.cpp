#include "renderers/painteroptions.h"

#include <QWidget>
#include <QPaintDevice>

namespace omm
{

PainterOptions::PainterOptions(const QWidget& viewport)
  : device_is_viewport(true), device(viewport)
{
}

PainterOptions::PainterOptions(const QPaintDevice& device)
  : device_is_viewport(false), device(device)
{
}

}  // namespace omm
