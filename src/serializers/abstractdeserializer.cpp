#include "serializers/abstractdeserializer.h"
#include "aspects/propertyowner.h"

namespace omm::serialization
{

void AbstractDeserializer::add_references(const std::set<AbstractPropertyOwner*>& references)
{
  for (AbstractPropertyOwner* reference : references) {
    register_reference(reference->id(), *reference);
  }
}

void AbstractDeserializer::polish()
{
  // polish reference properties
  for (const auto& polisher : m_reference_polishers) {
    polisher->update_references(m_id_to_reference);
  }
  m_reference_polishers.clear();
}

void AbstractDeserializer::register_reference(const std::size_t id,
                                              AbstractPropertyOwner& reference)
{
  const auto it = m_id_to_reference.find(id);
  if (it == m_id_to_reference.end()) {
    m_id_to_reference.insert({id, &reference});
  } else {
    if (it->second != &reference) {
      const QString msg = "Ambiguous id: %1.";
      // Unfortunately, it's not clear if the objects have already been deserialized.
      // Hence it does not make much sense to print their names.
      // User must search for the id in the json file to resolve the issue.
      throw DeserializeError(msg.arg(id).toStdString().c_str());
    }
  }
}

void AbstractDeserializer::register_reference_polisher(std::unique_ptr<ReferencePolisher> polisher)
{
  m_reference_polishers.insert(std::move(polisher));
}

}  // namespace omm::serialization
