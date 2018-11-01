#pragma once

#include <QMimeData>

namespace omm
{

class Object;

class ObjectMimeData : public QMimeData
{
  Q_OBJECT

public:
  const std::vector<std::reference_wrapper<Object>> objects;

  static constexpr auto MIME_TYPE = "application/objecttree-drag-drop-object";

  ObjectMimeData(const std::vector<std::reference_wrapper<Object>>& object);
  bool hasFormat(const QString& mimeType) const override;
  QStringList formats() const override;
  QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;
};

}  // namespace omm
