#pragma once

#include "managers/manager.h"
#include "widgets/numericedit.h"
#include "aspects/propertyowner.h"

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
  enum class Align { Local = 0, Global = 1 };
  Mode current_mode() const;
  Align current_align() const;

  struct UiBoundingBoxManagerDeleter
  {
    void operator()(::Ui::BoundingBoxManager* ui);
  };

  std::unique_ptr<::Ui::BoundingBoxManager, UiBoundingBoxManagerDeleter> m_ui;

private Q_SLOTS:
  void update_bounding_box();

};

}  // namespace omm
