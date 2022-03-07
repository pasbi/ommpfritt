#include "serializers/bin/binserializerworker.h"

#include "geometry/polarcoordinates.h"
#include "serializers/abstractserializer.h"
#include <QDataStream>

namespace omm::serialization
{

std::unique_ptr<SerializationArray> BinSerializerWorker::start_array(const std::size_t size)
{
  m_stream << static_cast<quint64>(size);
  return std::make_unique<SerializationArray>(*this);
}

BinSerializerWorker::BinSerializerWorker(QDataStream& stream)
    : m_stream(stream)
{
}

void BinSerializerWorker::set_value(int value)
{
  m_stream << value;
}

void BinSerializerWorker::set_value(bool value)
{
  m_stream << value;
}

void BinSerializerWorker::set_value(double value)
{
  m_stream << value;
}

void BinSerializerWorker::set_value(const QString& value)
{
  m_stream << value;
}

void BinSerializerWorker::set_value(const std::size_t id)
{
  m_stream << static_cast<quint64>(id);
}

void BinSerializerWorker::set_value(const TriggerPropertyDummyValueType&)
{
}

std::unique_ptr<SerializerWorker> BinSerializerWorker::sub(const std::string& key)
{
  Q_UNUSED(key)
  return std::make_unique<BinSerializerWorker>(m_stream);
}

std::unique_ptr<SerializerWorker> BinSerializerWorker::sub(const std::size_t i)
{
  Q_UNUSED(i)
  return std::make_unique<BinSerializerWorker>(m_stream);
}

}  // namespace omm::serialization
