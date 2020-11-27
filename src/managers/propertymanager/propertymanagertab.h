#pragma once

#include <QWidget>
#include <memory>
#include <set>

class QVBoxLayout;

namespace omm
{
class Scene;
class Property;
class AbstractPropertyOwner;

class PropertyManagerTab : public QWidget
{
public:
  explicit PropertyManagerTab(const QString& text);
  ~PropertyManagerTab() override;
  void add_properties(Scene& scene,
                      const QString& key,
                      const std::map<AbstractPropertyOwner*, Property*>& property_map);
  void set_header_visible(bool visible);

private:
  QVBoxLayout* m_layout;
  QWidget* m_header;
};

}  // namespace omm
