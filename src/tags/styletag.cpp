#include "tags/styletag.h"

#include <QApplication>  // TODO only for icon testing
#include <QStyle> // TODO only for icon testing

namespace omm
{

std::string StyleTag::type() const
{
  return "StyleTag";
}

QIcon StyleTag::icon() const
{
  return QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);
}

}  // namespace omm
