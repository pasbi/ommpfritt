#pragma once

namespace omm
{

class Selectable
{
public:
  void set_selected(bool is_selected);
  void select();
  void deselect();
  bool is_selected() const;

private:
  bool m_is_selected = false;
};

}  // namespace omm
