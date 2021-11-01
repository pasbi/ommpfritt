#pragma once

#include "common.h"
#include <QMimeData>
#include <vector>

namespace omm
{

class AbstractPropertyOwner;
class Tag;
class Object;
class Style;

class PropertyOwnerMimeData : public QMimeData
{
  Q_OBJECT

public:
  static constexpr auto MIME_TYPE = "application/propertyowners";

  PropertyOwnerMimeData(const std::vector<AbstractPropertyOwner*>& items);
  [[nodiscard]] bool hasFormat(const QString& mimeType) const override;
  [[nodiscard]] QStringList formats() const override;
  [[nodiscard]] QVariant retrieveData(const QString& mimeType, QVariant::Type type) const override;

  [[nodiscard]] std::vector<Object*> objects() const;
  [[nodiscard]] std::vector<Tag*> tags() const;
  [[nodiscard]] std::vector<Style*> styles() const;
  [[nodiscard]] std::vector<AbstractPropertyOwner*> items(Kind kinds) const;
  template<typename T = AbstractPropertyOwner> std::vector<T*> items() const;

private:
  const std::vector<AbstractPropertyOwner*> m_items;
};

}  // namespace omm
