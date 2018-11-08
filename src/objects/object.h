#pragma once

#include <vector>
#include <memory>
#include "external/json_fwd.hpp"
#include "objecttransformation.h"
#include "properties/hasproperties.h"
#include "abstractfactory.h"
#include "objects/selectable.h"
#include "common.h"
#include "serializers/serializable.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Object
  : public HasProperties
  , public Selectable
  , public AbstractFactory<std::string, Object>
{
public:
  explicit Object();
  virtual ~Object();

  bool is_root() const;
  Object& parent() const;

  Object& adopt(std::unique_ptr<Object> object, const Object* predecessor);
  Object& adopt(std::unique_ptr<Object> object);
  std::unique_ptr<Object> repudiate(Object& object);

  void transform(const ObjectTransformation& transformation);
  ObjectTransformation transformation() const;
  ObjectTransformation global_transformation() const;
  void set_transformation(const ObjectTransformation& transformation);
  void set_global_transformation(const ObjectTransformation& globalTransformation);

  Tag& add_tag(std::unique_ptr<Tag> tag);
  std::unique_ptr<Tag> remove_tag(Tag& tag);

  std::unique_ptr<Object> copy() const;
  ObjectRefs children() const;
  Object& child(size_t i) const;
  size_t n_children() const;
  size_t row() const;
  const Object* predecessor() const;
  size_t get_insert_position(const Object* child_before_position) const;

  std::unordered_set<HasProperties*> get_selected_children_and_tags();

  void reset_parent(Object& new_parent);

  static ObjectTransformation translation(const double& dx, const double dy);
  static ObjectTransformation rotation(const double& r);
  static ObjectTransformation scalation(const double& sx, const double sy);
  static ObjectTransformation identity();

  static const std::string TRANSFORMATION_PROPERTY_KEY;
  static const std::string NAME_PROPERTY_KEY;
  static const std::string THE_ANSWER_KEY;

  static void register_objects();
  std::string type() const override;

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

private:
  std::vector<std::unique_ptr<Tag>> m_tags;
  std::vector<std::unique_ptr<Object>> m_children;
  Object* m_parent;

  friend class ObjectView;
};

std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm