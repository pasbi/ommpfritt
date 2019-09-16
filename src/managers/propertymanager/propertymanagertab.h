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
  void add_properties(Scene& scene, const std::string& key,
                      const std::set<AbstractPropertyOwner*> &owners,
                      const std::set<Property*>& properties);

private:
  QFormLayout* m_layout;
};

}  // namespace omm
