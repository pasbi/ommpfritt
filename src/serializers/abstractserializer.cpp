#include "serializers/abstractserializer.h"

#include <numeric>
#include <ostream>
#include <istream>

#include "scene/scene.h"
#include "tags/tag.h"
#include "objects/object.h"

namespace
{

omm::Serializing::Key join(const std::vector<omm::Serializing::Key>& keys)
{
  const auto f = [](const omm::Serializing::Key& accu, const omm::Serializing::Key& key) {
    constexpr auto separator = "/";
    return accu + separator + key;
  };
  return std::accumulate(keys.begin(), keys.end(), keys.front(), f);
}

}  // namespace

namespace omm
{

namespace Serializing
{

AbstractSerializer::AbstractSerializer(const Scene& scene)
  : m_scene(scene)
{

}

AbstractSerializer::~AbstractSerializer()
{

}

std::ostream& AbstractSerializer::serialize(std::ostream& ostream)
{
  serialize(m_scene.root(), "root");
  // store data in overridden function
  return ostream;
}

void AbstractSerializer::serialize(const HasProperties& has_properties, const Key& key)
{
  for (const auto& property_key : has_properties.properties().keys()) {
    const auto& property = has_properties.property(key);
    set_type(property.type(), key);
    serialize( property, property_key, join({key, PROPERTIES_KEY, property_key}) );
  }
}

void AbstractSerializer::serialize(const Object& object, const Key& key)
{
  serialize(static_cast<const HasProperties&>(object), key);

  const auto children_key = join({key, CHILDREN_KEY});
  start_array(object.n_children(), children_key);
  for (const Object& child : object.children()) {
    serialize(child, children_key);
  }
  end_array();
}

void AbstractSerializer::serialize(const Tag& tag, const Key& key)
{
  serialize(static_cast<const HasProperties&>(tag), key);
}

AbstractDeserializer::AbstractDeserializer(Scene& scene)
  : m_scene(scene)
{

}

AbstractDeserializer::~AbstractDeserializer()
{

}

std::istream& AbstractDeserializer::deserialize(std::istream& istream)
{
  // load data in overridden function
  m_scene.replace_root(deserialize_object("root"));
  return istream;
}

std::unique_ptr<Object> AbstractDeserializer::deserialize_object(const Key& key)
{
  auto object = Object::make(get_type(join({TYPE_KEY, key})));

  deserialize(static_cast<HasProperties&>(*object), key);

  const auto children_key = join({key, CHILDREN_KEY});
  size_t n_children = array_size(children_key);
  for (size_t i = 0; i < n_children; ++i) {
    object->adopt(deserialize_object(children_key));
  }

  const auto tags_key = join({key, TAGS_KEY});
  size_t n_tags = array_size(tags_key);
  for (size_t i = 0; i < n_tags; ++i) {
    object->add_tag(deserialize_tag(tags_key));
  }

  return object;
}

std::unique_ptr<Tag> AbstractDeserializer::deserialize_tag(const Key& key)
{
  auto tag = Tag::make(get_type(join({TYPE_KEY, key})));
  deserialize(static_cast<HasProperties&>(*tag), key);
  return tag;
}

void AbstractDeserializer::deserialize(HasProperties& has_properties, const Key& key)
{
  const auto property_key = join({key, PROPERTIES_KEY});
  size_t n_properties = array_size(property_key);
  for (size_t i = 0; i < n_properties; ++i) {
    const auto type = get_type(key);
    std::unique_ptr<Property> property = Property::make(type);
    PropertyMap::key_type key;
    deserialize(*property, key, property_key);
    has_properties.add_property(key, std::move(property));
  }
}

void AbstractSerializer::register_serializers()
{
#define REGISTER_SERIALIZER(TYPE) AbstractSerializer::register_type<TYPE>(#TYPE);
  // REGISTER_SERIALIZER(JSONSerializer);
#undef REGISTER_SERIALIZER
}

void AbstractDeserializer::register_deserializers()
{
#define REGISTER_DESERIALIZER(TYPE) REGISTER_DESERIALIZER::register_type<TYPE>(#TYPE);
  // REGISTER_DESERIALIZER(JSONDeserializer);
#undef REGISTER_DESERIALIZER
}

}  // namespace Serializing

}  // namespace omm