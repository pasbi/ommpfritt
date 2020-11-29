#pragma once

#include <map>
#include <QString>
#include <variant>
#include "geometry/vec2.h"
#include <cctype>
#include "properties/propertyfilter.h"
#include "serializers/abstractserializer.h"

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
                            std::vector<QString>,
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

  const value_type& get(const QString& key) const;

  template<typename T> T get(const QString& key, const T& default_value) const
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
    if constexpr (std::is_enum_v<T>) {
      set<std::size_t>(key, static_cast<std::size_t>(value));
    } else {
      m_store.insert(std::pair{key, std::forward<T>(value)});
    }
  }

  template<typename T>
  void deserialize_field(const QString& field,
                         AbstractDeserializer& deserializer,
                         const Serializable::Pointer& root)
  {
    if (m_store.find(field) == m_store.end()) {
      (m_store)[field] = deserializer.get<T>(Serializable::make_pointer(root, field));
    }
  }

  std::size_t count(const QString& key) const;
  map_type::const_iterator find(const QString& key) const;
  map_type::const_iterator end() const;
};

}  // namespace omm
