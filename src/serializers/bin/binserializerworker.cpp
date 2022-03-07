#include "serializers/bin/binserializerworker.h"

#include "geometry/polarcoordinates.h"
#include "serializers/abstractserializer.h"
#include <QDataStream>
#include <typeinfo>

#if 0
#define DEBUG_WRITE(value, type) \
    LDEBUG << "serialize "#type" " << typeid(value).name() << " " << value
#else
#define DEBUG_WRITE(value, type)
#endif

namespace omm::serialization
{

std::unique_ptr<SerializationArray> BinSerializerWorker::start_array(const std::size_t size)
{
  set_value(size);
  return std::make_unique<SerializationArray>(*this);
}

BinSerializerWorker::BinSerializerWorker(QDataStream& stream)
    : m_stream(stream)
{
}

void BinSerializerWorker::set_value(int value)
{
  DEBUG_WRITE(value, int);
  m_stream << value;
}

void BinSerializerWorker::set_value(bool value)
{
  DEBUG_WRITE(value, bool);
  m_stream << value;
}

void BinSerializerWorker::set_value(double value)
{
  DEBUG_WRITE(value, double);
  m_stream << value;
}

void BinSerializerWorker::set_value(const QString& value)
{
  DEBUG_WRITE(value, QString);
  m_stream << value;
}

void BinSerializerWorker::set_value(const std::size_t value)
{
  DEBUG_WRITE(value, std::size_t);
  m_stream << static_cast<quint64>(value);
}

void BinSerializerWorker::set_value(const TriggerPropertyDummyValueType&)
{
  DEBUG_WRITE("", TriggerPropertyDummyValueType);
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
