#include "aspects/selectable.h"
#include <glog/logging.h>
#include <cassert>

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
  set_selected(true);
}

void Selectable::deselect()
{
  set_selected(false);
}

}  // namespace omm
