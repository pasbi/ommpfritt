#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "tags/tag.h"

namespace
{
std::vector<omm::AbstractPropertyOwner*>
filter(const std::vector<omm::AbstractPropertyOwner*>& items, omm::Kind kinds)
{
  std::vector<omm::AbstractPropertyOwner*> filtered;
  for (omm::AbstractPropertyOwner* item : items) {
    if (!!(item->kind & kinds)) {
      filtered.push_back(item);
    }
  }
  return filtered;
}

template<typename ItemType>
std::vector<ItemType*> filter(const std::vector<omm::AbstractPropertyOwner*>& items)
{
  const auto filtered = filter(items, ItemType::KIND);
  return util::transform(filtered, [](auto item) { return static_cast<ItemType*>(item); });
}

}  // namespace

namespace omm
{
PropertyOwnerMimeData::PropertyOwnerMimeData(const std::vector<AbstractPropertyOwner*>& items)
    : m_items(items)
{
}

bool PropertyOwnerMimeData::hasFormat(const QString& mimeType) const
{
  return mimeType == MIME_TYPE;
}

QStringList PropertyOwnerMimeData::formats() const
{
  return {MIME_TYPE};
}

QVariant PropertyOwnerMimeData::retrieveData(const QString& mimeType, QVariant::Type type) const
{
  Q_UNUSED(mimeType)
  Q_UNUSED(type)
  LFATAL("This function shall not be called.");
  return QVariant();
}

std::vector<Object*> PropertyOwnerMimeData::objects() const
{
  return filter<Object>(m_items);
}

std::vector<Tag*> PropertyOwnerMimeData::tags() const
{
  return filter<Tag>(m_items);
}

std::vector<Style*> PropertyOwnerMimeData::styles() const
{
  return filter<Style>(m_items);
}

std::vector<AbstractPropertyOwner*> PropertyOwnerMimeData::items(Kind kinds) const
{
  return filter(m_items, kinds);
}

template<> std::vector<Object*> PropertyOwnerMimeData::items<Object>() const
{
  return objects();
}

template<> std::vector<Style*> PropertyOwnerMimeData::items<Style>() const
{
  return styles();
}

template<>
std::vector<AbstractPropertyOwner*> PropertyOwnerMimeData::items<AbstractPropertyOwner>() const
{
  return m_items;
}

template<> std::vector<Tag*> PropertyOwnerMimeData::items<Tag>() const
{
  return tags();
}

}  // namespace omm
