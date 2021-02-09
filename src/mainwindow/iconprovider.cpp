#include "iconprovider.h"
#include "aspects/propertyowner.h"
#include "renderers/style.h"
#include "renderers/styleiconengine.h"
#include "tags/styletag.h"
#include "tags/tag.h"

namespace omm
{
QIcon IconProvider::icon(const AbstractPropertyOwner& owner)
{
  if (const Tag* tag = kind_cast<const Tag*>(&owner); tag != nullptr) {
    if (const auto* style_tag = type_cast<const StyleTag*>(tag); style_tag != nullptr) {
      const Property* rprop = style_tag->property(StyleTag::STYLE_REFERENCE_PROPERTY_KEY);
      const auto* style = kind_cast<const Style*>(rprop->value<AbstractPropertyOwner*>());
      return QIcon(std::make_unique<StyleIconEngine>(style).release());
    }
  } else if (const auto* style = kind_cast<const Style*>(&owner); style != nullptr) {
    return QIcon(std::make_unique<StyleIconEngine>(style).release());
  }
  return pixmap(owner.type());
}

QPixmap IconProvider::pixmap(const QString& name, Size size)
{
  return pixmap(name, Orientation::Normal, size);
}

QPixmap IconProvider::pixmap(const QString& name, Orientation orientation, Size size)
{
  constexpr auto icon_path_pattern = ":/icons/%1_%2.png";
  const int resolution = static_cast<std::underlying_type_t<Size>>(size);
  const auto fn = QString(icon_path_pattern).arg(name).arg(resolution);
  if (orientation == Orientation::Normal) {
    return QPixmap(fn);
  } else {
    const auto h = static_cast<bool>(orientation & Orientation::FlippedHorizontally);
    const auto v = static_cast<bool>(orientation & Orientation::FlippedVertically);
    return QPixmap::fromImage(QImage(fn).mirrored(h, v));
  }
}

}  // namespace omm
