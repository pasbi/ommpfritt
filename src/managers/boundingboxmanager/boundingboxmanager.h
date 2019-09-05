#pragma once

#include "managers/manager.h"
#include "widgets/numericedit.h"
#include "aspects/propertyowner.h"
#include "geometry/boundingbox.h"

namespace Ui { class BoundingBoxManager; }

namespace omm
{

class AbstractPropertyOwner;

class BoundingBoxManager : public Manager
{
  Q_OBJECT
public:
  BoundingBoxManager(Scene &scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "BoundingBoxManager");

public Q_SLOTS:
  void on_property_value_changed(Property& property);

private:
  enum class Mode { Points = 0, Objects = 1 };
  Mode current_mode() const;
  void update_points();
  void update_objects();
  BoundingBox bounding_box() const;
  BoundingBox m_old_bounding_box;

  struct UiBoundingBoxManagerDeleter
  {
    void operator()(::Ui::BoundingBoxManager* ui);
  };

  std::unique_ptr<::Ui::BoundingBoxManager, UiBoundingBoxManagerDeleter> m_ui;

private Q_SLOTS:
  void update_manager();
  void update_bounding_box();

  void block_signals();
  void unblock_signals();

};

}  // namespace omm
