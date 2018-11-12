#pragma once

#include <QWidget>
#include <set>

class QVBoxLayout;

namespace omm
{

class Scene;
class Property;

class PropertyManagerTab : public QWidget
{
public:
  explicit PropertyManagerTab();
  void add_properties(Scene& scene, const std::set<Property*>& properties);
  void end_add_properties();

private:
  QVBoxLayout* m_layout;
};

}  // namespace omm
