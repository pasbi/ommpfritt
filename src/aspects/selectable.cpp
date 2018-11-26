#include "aspects/selectable.h"
#include <glog/logging.h>

namespace omm
{

void Selectable::set_selected(bool is_selected)
{
  m_is_selected = is_selected;
}

bool Selectable::is_selected() const
{
  return m_is_selected;
}

void Selectable::select()
{
  m_is_selected = true;
}

void Selectable::deselect()
{
  m_is_selected = false;
}

}  // namespace omm
