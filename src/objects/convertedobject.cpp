#include "objects/convertedobject.h"
#include "objects/object.h"
#include "scene/disjointpathpointsetforest.h"

namespace omm
{

ConvertedObject::ConvertedObject(std::unique_ptr<Object>&& object, bool keep_children)
  : ConvertedObject(std::move(object), {}, keep_children)
{

}

ConvertedObject::ConvertedObject(std::unique_ptr<Object>&& object,
                                 std::unique_ptr<DisjointPathPointSetForest>&& joined_points,
                                 bool keep_children)
  : object(std::move(object))
  , joined_points(std::move(joined_points))
  , keep_children(keep_children)
{

}

ConvertedObject::ConvertedObject(std::unique_ptr<Object>&& object,
                                 DisjointPathPointSetForest&& joined_points,
                                 bool keep_children)
  : object(std::move(object))
  , joined_points(std::make_unique<DisjointPathPointSetForest>(joined_points))
  , keep_children(keep_children)
{
}

ConvertedObject::~ConvertedObject() = default;

}  // namespace omm
