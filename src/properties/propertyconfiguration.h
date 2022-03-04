#pragma once

#include "geometry/vec2.h"
#include "properties/propertyfilter.h"
#include <QString>
#include <cctype>
#include <map>
#include <variant>

namespace omm
{

class PropertyConfiguration
{
public:
  using value_type = std::variant<bool,
                                  int,
                                  double,
                                  Vec2i,
                                  Vec2f,
                                  std::size_t,
                                  QString,
                                  std::deque<QString>,
                                  PropertyFilter>;

private:
  using map_type = std::map<QString, value_type>;
  map_type m_store;

public:
  template<typename T> [[nodiscard]] T get(const QString& key) const
  {
    if constexpr (std::is_enum_v<T>) {
      return static_cast<T>(get<std::size_t>(key));
    } else {
      const auto cit = m_store.find(key);
      assert(cit != m_store.end());

      const T* value = std::get_if<T>(&cit->second);
      assert(value != nullptr);
      return *value;
    }
  }

  [[nodiscard]] const value_type& get(const QString& key) const;

  template<typename T> [[nodiscard]] T get(const QString& key, const T& default_value) const
  {
    if constexpr (std::is_enum_v<T>) {
      return static_cast<T>(get<std::size_t>(key, default_value));
    } else {
      const auto cit = m_store.find(key);
      if (cit == m_store.end()) {
        return default_value;
      } else {
        const T* value = std::get_if<T>(&cit->second);
        if (value != nullptr) {
          return *value;
        } else {
          return default_value;
        }
      }
    }
  }

  template<typename T> void set(const QString& key, T&& value)
  {
    if constexpr (std::is_enum_v<std::decay_t<T>>) {
      set<std::size_t>(key, static_cast<std::size_t>(value));
    } else {
      m_store[key] = std::forward<T>(value);
    }
  }

  template<typename T>
  void deserialize_field(const QString& field, serialization::DeserializerWorker& worker)
  {
    if (m_store.find(field) == m_store.end()) {
      m_store[field] = worker.sub(field.toStdString())->get<T>();
    }
  }

  [[nodiscard]] std::size_t count(const QString& key) const;
  [[nodiscard]] map_type::const_iterator find(const QString& key) const;
  [[nodiscard]] map_type::const_iterator end() const;
};

}  // namespace omm
