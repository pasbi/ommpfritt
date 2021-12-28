#pragma once

#include "geometry/boundingbox.h"
#include "managers/manager.h"
#include "tools/transformpointshelper.h"
#include "tools/transformobjectshelper.h"

namespace Ui
{
class BoundingBoxManager;
}

namespace omm
{

class Property;
class BoundingBoxManager : public Manager
{
  Q_OBJECT
public:
  BoundingBoxManager(Scene& scene);
  ~BoundingBoxManager() override;
  BoundingBoxManager(const BoundingBoxManager&) = delete;
  BoundingBoxManager(BoundingBoxManager&&) = delete;
  BoundingBoxManager& operator=(const BoundingBoxManager&) = delete;
  BoundingBoxManager& operator=(BoundingBoxManager&&) = delete;
  [[nodiscard]] QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "BoundingBoxManager");

public:
  void on_property_value_changed(omm::Property& property);

private:
  enum class Mode { Points = 0, Objects = 1 };
  [[nodiscard]] BoundingBox bounding_box() const;
  BoundingBox m_old_bounding_box;

  std::unique_ptr<::Ui::BoundingBoxManager> m_ui;
  TransformPointsHelper m_transform_points_helper;
  TransformObjectsHelper m_transform_objects_helper;
  Mode m_current_mode = Mode::Objects;
  bool perform_action(const QString& name) override;

protected:
  void enterEvent(QEvent* e) override;
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  omm::BoundingBox update_manager();
  void update_bounding_box();
  void reset_transformation();

  std::vector<std::unique_ptr<QSignalBlocker>> acquire_signal_blockers();
};

}  // namespace omm
