#pragma once

#include "managers/manager.h"
#include "widgets/numericedit.h"
#include "aspects/propertyowner.h"
#include "geometry/boundingbox.h"
#include "tools/selectpointstool.h"
#include "tools/selectobjectstool.h"
#include "aspects/autoconnectiondeleter.h"

namespace Ui { class BoundingBoxManager; }

namespace omm
{

class AbstractPropertyOwner;

class BoundingBoxManager : public Manager, public AutoConnectionDeleter
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
  BoundingBox bounding_box() const;
  BoundingBox m_old_bounding_box;

  struct UiBoundingBoxManagerDeleter
  {
    void operator()(::Ui::BoundingBoxManager* ui);
  };

  std::unique_ptr<::Ui::BoundingBoxManager, UiBoundingBoxManagerDeleter> m_ui;
  TransformPointsHelper m_transform_points_helper;
  TransformObjectsHelper m_transform_objects_helper;
  Mode m_current_mode = Mode::Objects;

protected:
  void enterEvent(QEvent* e) override;
  bool eventFilter(QObject *o, QEvent *e) override;

private Q_SLOTS:
  BoundingBox update_manager();
  void update_bounding_box();
  void reset_transformation();

  void block_signals();
  void unblock_signals();
};

}  // namespace omm
