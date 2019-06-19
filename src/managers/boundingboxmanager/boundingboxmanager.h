#pragma once

#include "managers/manager.h"

namespace omm
{

class BoundingBoxManager : public Manager
{
  Q_OBJECT
public:
  BoundingBoxManager(Scene &scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "BoundingBoxManager");
};

}  // namespace omm
