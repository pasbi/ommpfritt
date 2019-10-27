#pragma once

#include "serializers/abstractserializer.h"
#include "external/json.hpp"
#include "color/color.h"

namespace omm
{

class JSONSerializer : public AbstractSerializer
{
public:
  explicit JSONSerializer(std::ostream& ostream);
  ~JSONSerializer();

  void start_array(size_t size, const Pointer& pointer) override;
  void end_array() override;
  void set_value(int value, const Pointer& pointer) override;
  void set_value(bool value, const Pointer& pointer) override;
  void set_value(double value, const Pointer& pointer) override;
  void set_value(const QString& value, const Pointer& pointer) override;
  void set_value(const std::size_t id, const Pointer& pointer) override;
  void set_value(const Color& color, const Pointer& pointer) override;
  void set_value(const Vec2f& value, const Pointer& pointer) override;
  void set_value(const Vec2i& value, const Pointer& pointer) override;
  void set_value(const PolarCoordinates& value, const Pointer& pointer) override;

  QString type() const override;

private:
  nlohmann::json m_store;
  std::ostream& m_ostream;
};

class JSONDeserializer : public AbstractDeserializer
{
public:
  explicit JSONDeserializer(std::istream& istream);

  // there is no virtual template, unfortunately: https://stackoverflow.com/q/2354210/4248972
  size_t array_size(const Pointer& pointer) override;
  int get_int(const Pointer& pointer) override;
  double get_double(const Pointer& pointer) override;
  bool get_bool(const Pointer& pointer) override;
  QString get_string(const Pointer& pointer) override;
  Color get_color(const Pointer& pointer) override;
  std::size_t get_size_t(const Pointer& pointer) override;
  Vec2f get_vec2f(const Pointer& pointer) override;
  Vec2i get_vec2i(const Pointer& pointer) override;
  PolarCoordinates get_polarcoordinates(const Pointer& pointer) override;

  QString type() const override;

private:
  nlohmann::json m_store;
};

}  // namespace omm

