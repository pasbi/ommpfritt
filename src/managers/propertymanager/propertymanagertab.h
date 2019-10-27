#pragma once

#include <QWidget>
#include <set>
#include <memory>

class QFormLayout;

namespace omm
{

class Scene;
class Property;
class AbstractPropertyOwner;

class PropertyManagerTab : public QWidget
{
public:
  explicit PropertyManagerTab();
  ~PropertyManagerTab();
  void add_properties(Scene& scene, const QString& key,
                      const std::map<AbstractPropertyOwner*, Property*> &property_map);

private:
  QFormLayout* m_layout;
};

}  // namespace omm
