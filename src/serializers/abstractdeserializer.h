#pragma once

#include <QObject>
#include <memory>
#include <set>

namespace omm
{
class AbstractPropertyOwner;
}  // namespace omm

namespace omm::serialization
{

class ReferencePolisher
{
public:
  virtual ~ReferencePolisher() = default;
  ReferencePolisher() = default;
  ReferencePolisher(ReferencePolisher&&) = default;
  ReferencePolisher(const ReferencePolisher&) = default;
  ReferencePolisher& operator=(ReferencePolisher&&) = default;
  ReferencePolisher& operator=(const ReferencePolisher&) = default;

protected:
  virtual void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) = 0;
  friend class AbstractDeserializer;
};

class DeserializerWorker;
class AbstractDeserializer : public QObject
{
  Q_OBJECT
public:
  explicit AbstractDeserializer() = default;

  /**
   * @brief polish sets all registered references.
   */
  void polish();

  void add_references(const std::set<AbstractPropertyOwner*>& existing_references);

  class DeserializeError : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };

  void register_reference(std::size_t id, AbstractPropertyOwner& reference);
  void register_reference_polisher(std::unique_ptr<ReferencePolisher> polisher);
  virtual std::unique_ptr<DeserializerWorker> worker() = 0;

private:
  // maps old stored hash to new ref
  std::map<std::size_t, AbstractPropertyOwner*> m_id_to_reference;
  std::set<std::unique_ptr<ReferencePolisher>> m_reference_polishers;
};

}  // namespace omm::serialization
