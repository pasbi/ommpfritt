#pragma once

#include <iosfwd>
#include "abstractfactory.h"
#include "properties/propertymap.h"

namespace omm
{

class Scene;
class Object;
class Property;
class Tag;
class HasProperties;

namespace Serializing
{

using Key = std::string;
constexpr auto PROPERTIES_KEY = "properties";
constexpr auto CHILDREN_KEY = "children";
constexpr auto TAGS_KEY = "children";
constexpr auto TYPE_KEY = "type";


class AbstractSerializer : public AbstractFactory<std::string, AbstractSerializer, Scene&>
{
public:
  explicit AbstractSerializer(const Scene& scene);
  virtual ~AbstractSerializer();
  std::ostream& serialize(std::ostream& ostream);
  static void register_serializers();

protected:
  virtual void serialize( const Property& property, const PropertyMap::key_type& property_key,
                          const Key& key ) = 0;
  virtual void start_array(size_t size, const Key& key) = 0;
  virtual void end_array() = 0;
  virtual void set_type(const std::string& type, const Key& key) = 0;

private:
  virtual void serialize(const Object& object, const Key& key);
  virtual void serialize(const Tag& tag, const Key& key);
  virtual void serialize(const HasProperties& has_properties, const Key& key);

private:
  const Scene& m_scene;
};

class AbstractDeserializer : public AbstractFactory<std::string, AbstractDeserializer, Scene&>
{
public:
  explicit AbstractDeserializer(Scene& scene);
  virtual ~AbstractDeserializer();
  virtual void load(const std::string& filename) = 0;
  std::istream& deserialize(std::istream& istream);
  static void register_deserializers();

protected:
  /**
   * @brief deserialize the property and its property key at key
   */
  virtual void deserialize( Property& property, PropertyMap::key_type& property_key,
                            const Key& key ) = 0;

  /**
   * @return the size of the array at key
   */
  virtual size_t array_size(const Key& key) = 0;

  /**
   * @return returns the type of the next object at key
   */
  virtual std::string get_type(const Key& key) = 0;

private:
  void deserialize(HasProperties& has_properties, const Key& key);
  std::unique_ptr<Object> deserialize_object(const Key& key);
  std::unique_ptr<Tag> deserialize_tag(const Key& key);

private:
  Scene& m_scene;
};

}  // namespace Serializing

}  // namespace omm

