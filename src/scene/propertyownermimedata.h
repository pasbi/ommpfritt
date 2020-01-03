#pragma once

#include <QMimeData>
#include "aspects/propertyowner.h"

namespace omm
{

class Tag;
class Object;
class Style;

class PropertyOwnerMimeData : public QMimeData
{
  Q_OBJECT

public:

  static constexpr auto MIME_TYPE = "application/propertyowners";

  PropertyOwnerMimeData(const std::vector<AbstractPropertyOwner*>& items);
  bool hasFormat(const QString& mimeType) const override;
  QStringList formats() const override;
  QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;

  std::vector<Object*> objects() const;
  std::vector<Tag*> tags() const;
  std::vector<Style*> styles() const;
  std::vector<AbstractPropertyOwner*> items(Kind kinds) const;
  template<typename T=AbstractPropertyOwner> std::vector<T*> items() const;
private:
  const std::vector<AbstractPropertyOwner*> m_items;
};

}  // namespace omm
