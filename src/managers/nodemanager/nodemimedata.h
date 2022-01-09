#pragma once

#include "logging.h"
#include "nodesystem/common.h"
#include <QMimeData>

namespace omm
{

namespace nodes
{
class Node;
class InputPort;
class OutputPort;
}  // namespace nodes

class NodeMimeData : public QMimeData
{
  Q_OBJECT

public:
  static const std::map<nodes::BackendLanguage, QString> MIME_TYPES;
  NodeMimeData(nodes::BackendLanguage language, const std::set<nodes::Node*>& items);
  [[nodiscard]] bool hasFormat(const QString& mimeType) const override;
  [[nodiscard]] QStringList formats() const override
  {
    return {MIME_TYPES.at(m_language)};
  }
  [[noreturn]] QVariant retrieveData(const QString&, QVariant::Type) const override;
  [[nodiscard]] std::set<nodes::Node*> nodes() const
  {
    return m_nodes;
  }

private:
  const nodes::BackendLanguage m_language;
  std::set<nodes::Node*> m_nodes;
};

}  // namespace omm
