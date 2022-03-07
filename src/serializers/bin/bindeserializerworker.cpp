#include "serializers/bin/bindeserializerworker.h"

#include "geometry/polarcoordinates.h"
#include "serializers/abstractdeserializer.h"
#include <QDataStream>
#include <typeinfo>

#if 0
#define DEBUG_READ(value, type) \
    LDEBUG << "deserialize "#type" " << typeid(value).name() << " " << value
#else
#define DEBUG_READ(value, type)
#endif


namespace omm::serialization
{

BinDeserializerWorker::BinDeserializerWorker(AbstractDeserializer& deserializer, QDataStream& stream)
    : DeserializerWorker(deserializer)
    , m_stream(stream)
{
}

int BinDeserializerWorker::get_int()
{
  qint32 value = 0;
  m_stream >> value;
  DEBUG_READ(value, int);
  return value;
}

bool BinDeserializerWorker::get_bool()
{
  bool value = false;
  m_stream >> value;
  DEBUG_READ(value, bool);
  return value;
}

double BinDeserializerWorker::get_double()
{
  double value = 0.0;
  m_stream >> value;
  DEBUG_READ(value, double);
  return value;
}

QString BinDeserializerWorker::get_string()
{
  QString value = "";
  m_stream >> value;
  DEBUG_READ(value, QString);
  return value;
}

std::size_t BinDeserializerWorker::get_size_t()
{
  quint64 value = 0;
  m_stream >> value;
  DEBUG_READ(value, std::size_t);
  return static_cast<std::size_t>(value);
}

TriggerPropertyDummyValueType BinDeserializerWorker::get_trigger_dummy_value()
{
  DEBUG_READ("", TriggerPropertyDummyValueType);
  return {};
}

std::unique_ptr<DeserializationArray> BinDeserializerWorker::start_array()
{
  return std::make_unique<DeserializationArray>(*this, get_size_t());
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
