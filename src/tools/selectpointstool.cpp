#include "tools/selectpointstool.h"
#include "keybindings/keybindings.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "properties/optionproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "scene/pointselection.h"
#include "tools/transformpointshelper.h"
#include <QMouseEvent>

namespace omm
{

QString SelectPointsTool::type() const
{
  return TYPE;
}

}  // namespace omm
