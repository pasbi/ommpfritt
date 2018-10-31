#pragma once

#include <QWidget>
#include <vector>

namespace omm
{

class Property;

class PropertyManagerTab : public QWidget
{
public:
  explicit PropertyManagerTab();
  void add_properties(const std::vector<Property*>& properties);
};

}  // namespace omm
