#pragma once

#include <QScrollArea>
#include <set>
#include <memory>

class QFormLayout;

namespace omm
{

class Scene;
class Property;

class PropertyManagerTab : public QScrollArea
{
public:
  explicit PropertyManagerTab();
  ~PropertyManagerTab();
  void add_properties(Scene& scene, const std::string& key, const std::set<Property*>& properties);
  void end_add_properties();

private:
  QFormLayout* m_layout;
  std::unique_ptr<QWidget> m_widget;
};

}  // namespace omm
