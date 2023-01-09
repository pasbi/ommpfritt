#pragma once

#include "tools/selecttool.h"

namespace omm
{

class SelectFacesTool : public AbstractSelectTool
{
  Q_OBJECT
public:
  using AbstractSelectTool::AbstractSelectTool;
  QString type() const override;
  SceneMode scene_mode() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectFacesTool");
  Vec2f selection_center() const override;
  void transform_objects(omm::ObjectTransformation transformation) override;
  void reset() override;

private:
  void make_handles();
};

}  // namespace
