#pragma once

#include "common.h"
#include <QIcon>

namespace omm
{
class AbstractPropertyOwner;

class IconProvider
{
public:
  explicit IconProvider() = default;

  enum class Size : int {
    Tiny = 16,
    Small = 22,
    Medium = 32,
    Ample = 48,
    Large = 64,
    Huge = 128,
    Gigantic = 2048,
  };

  enum class Orientation {
    Normal = 0x0,
    FlippedHorizontally = 0x1,
    FlippedVertically = 0x2,
    Flipped = 0x3
  };

  static QIcon icon(const AbstractPropertyOwner& owner);
  static QPixmap pixmap(const QString& name, Size size = Size::Huge);
  static QPixmap pixmap(const QString& name, Orientation orientation, Size size = Size::Huge);
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::IconProvider::Orientation> : std::true_type {
};
