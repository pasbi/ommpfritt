#include "managers/nodemanager/nodemimedata.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "tags/tag.h"

namespace omm
{
const std::map<nodes::BackendLanguage, QString> NodeMimeData::MIME_TYPES
    = {{nodes::BackendLanguage::Python, "application/pythonnodes"},
       {nodes::BackendLanguage::GLSL, "application/glslnodes"}};

QVariant NodeMimeData::retrieveData(const QString&, QVariant::Type) const
{
  LFATAL("This function shall not be called.");
  abort();
}

NodeMimeData::NodeMimeData(nodes::BackendLanguage language, const std::set<nodes::Node*>& items)
    : m_language(language), m_nodes(items)
{
}

bool NodeMimeData::hasFormat(const QString& mimeType) const
{
  return mimeType == MIME_TYPES.at(m_language);
}

}  // namespace omm
