#include "serializers/bin/bindeserializerworker.h"

#include "geometry/polarcoordinates.h"
#include "serializers/abstractdeserializer.h"
#include <QDataStream>


namespace omm::serialization
{

BinDeserializerWorker::BinDeserializerWorker(AbstractDeserializer& deserializer, QDataStream& stream)
    : DeserializerWorker(deserializer)
    , m_stream(stream)
{
}

int BinDeserializerWorker::get_int()
{
  qint32 i;
  m_stream >> i;
  return i;
}

bool BinDeserializerWorker::get_bool()
{
  bool b;
  m_stream >> b;
  return b;
}

double BinDeserializerWorker::get_double()
{
  double v;
  m_stream >> v;
  return v;
}

QString BinDeserializerWorker::get_string()
{
  QString s;
  m_stream >> s;
  return s;
}

std::size_t BinDeserializerWorker::get_size_t()
{
  quint64 i;
  m_stream >> i;
  return static_cast<std::size_t>(i);
}

TriggerPropertyDummyValueType BinDeserializerWorker::get_trigger_dummy_value()
{
  return {};
}

std::unique_ptr<DeserializationArray> BinDeserializerWorker::start_array()
{
  quint64 n;
  m_stream >> n;
  return std::make_unique<DeserializationArray>(*this, n);
}

std::unique_ptr<DeserializerWorker> BinDeserializerWorker::sub(const std::string& key)
{
  Q_UNUSED(key)
  return std::make_unique<BinDeserializerWorker>(deserializer(), m_stream);
}

std::unique_ptr<DeserializerWorker> BinDeserializerWorker::sub(const std::size_t i)
{
  Q_UNUSED(i)
  return std::make_unique<BinDeserializerWorker>(deserializer(), m_stream);
}

}  // namespace omm::serialization
