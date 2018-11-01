#pragma once

#include <QWidget>
#include <unordered_set>

namespace omm
{

class Property;

class PropertyManagerTab : public QWidget
{
public:
  explicit PropertyManagerTab();
  void add_properties(const std::unordered_set<Property*>& properties);
};

}  // namespace omm
