#include "iconprovider.h"
#include "aspects/propertyowner.h"
#include "renderers/style.h"
#include "renderers/styleiconengine.h"
#include "tags/styletag.h"
#include "tags/tag.h"

namespace omm
{
QIcon IconProvider::icon(AbstractPropertyOwner& owner) const
{
  if (Tag* tag = kind_cast<Tag*>(&owner); tag != nullptr) {
    if (StyleTag* style_tag = type_cast<StyleTag*>(tag); style_tag != nullptr) {
      const Property* rprop = style_tag->property(StyleTag::STYLE_REFERENCE_PROPERTY_KEY);
      const Style* style = kind_cast<const Style*>(rprop->value<AbstractPropertyOwner*>());
      return QIcon(std::make_unique<StyleIconEngine>(style).release());
    }
  } else if (Style* style = kind_cast<Style*>(&owner); style != nullptr) {
    return QIcon(std::make_unique<StyleIconEngine>(style).release());
  }
  return icon(owner.type());
}

QIcon IconProvider::icon(const QString& name) const
{
  // Fallback: load icon file
  const QString filename = ":/icons/" + name + "_128.png";
  if (m_cached_icons_from_file.find(filename) == m_cached_icons_from_file.end()) {
    const QPixmap pixmap(filename);
    const auto icon = pixmap.isNull() ? QIcon() : QIcon(pixmap);
    m_cached_icons_from_file.insert(std::pair(filename, icon));
  }
  return m_cached_icons_from_file.at(filename);
}

}  // namespace omm
