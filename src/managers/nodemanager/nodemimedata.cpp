#include "managers/nodemanager/nodemimedata.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "renderers/style.h"

namespace omm
{

const std::map<AbstractNodeCompiler::Language, QString> NodeMimeData::MIME_TYPES = {
    { AbstractNodeCompiler::Language::Python, "application/pythonnodes" },
    { AbstractNodeCompiler::Language::GLSL,   "application/glslnodes" }
};

QVariant NodeMimeData::retrieveData(const QString& mimeType, QVariant::Type type) const
{
  {
    Q_UNUSED(mimeType)
    Q_UNUSED(type)
    LFATAL("This function shall not be called.");
    abort();
  }
}

NodeMimeData::NodeMimeData(AbstractNodeCompiler::Language language, const std::set<Node*>& items)
  : m_language(language), m_nodes(items)
{
}

bool NodeMimeData::hasFormat(const QString& mimeType) const
{
  return mimeType == MIME_TYPES.at(m_language);
}

}  // namespace omm
