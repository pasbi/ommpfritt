#pragma once

#include"common.h"
#include <QObject>
#include <set>

namespace omm
{

namespace nodes
{
class Node;
}  // namespace nodes

class AbstractPropertyOwner;
class Object;
class Property;
class Style;
class Tag;
class Tool;

class MailBox : public QObject
{
  Q_OBJECT
public:
  explicit MailBox();

Q_SIGNALS:
  /**
   * @brief tool_appearance_changed is emitted when the appearance of a tool has changed.
   * This signal forwards to @code appearance_changed().
   */
  void tool_appearance_changed(omm::Tool&);

  /**
   * @brief appearance_changed is emitted when the appearance of an object has changed.
   *  Usually, this signal is emitted at the end of the Object::update method.
   * This signal is not emitted when only the transformation of an object changed.
   * @see transformation_changed(Object&)
   * This signal forwards to @code appearance_changed().
   * If the specified object has a parent, this signal will forward to that very parent's
   * child_appearance_change signal.
   */
  void object_appearance_changed(omm::Object&);

  /**
   * @brief transformation_changed similar to appearance_changed, however, this signal is only
   *  emitted when the transformation of an object changed.
   * This signal forwards to @code appearance_changed().
   */
  void transformation_changed(omm::Object&);

  /**
   * @brief scene_appearance_changed is emitted when the appearance of the scene changed, i.e., if
   * it needs to be redrawn. Many signals of this class forward to this one. This is the weakest
   * signal, it is emitted very frequently.
   */
  void scene_appearance_changed();

  /**
   * @brief object_inserted is emitted when an object was inserted into the parent object
   * This signal forwards to @code appearance_changed().
   */
  void object_inserted(omm::Object& parent, omm::Object& object);

  /**
   * @brief object_inserted is emitted when an object was removed from the parent object.
   * This signal forwards to @code appearance_changed().
   */
  void object_removed(omm::Object& parent, omm::Object& object);

  /**
   * @brief object_inserted is emitted when an object was moved from old_parent to new_parent.
   * This signal forwards to @code appearance_changed().
   */
  void object_moved(omm::Object& old_parent, omm::Object& new_parent, omm::Object& object);

  /**
   * @brief style_inserted is emitted when a style was inserted into the scene.
   * This signal forwards to @code appearance_changed().
   */
  void style_inserted(omm::Style&);

  /**
   * @brief style_removed is emitted when a style was removed from the scene.
   * This signal forwards to @code appearance_changed().
   */
  void style_removed(omm::Style&);

  /**
   * @brief style_moved is emitted when a style was moved.
   */
  void style_moved(omm::Style&);

  /**
   * @brief tag_inserted is emitted when a tag was attached to the object.
   * This signal forwards to @code appearance_changed().
   */
  void tag_inserted(omm::Object&, omm::Tag&);

  /**
   * @brief tag_removed is emitted when a tag was removed from the object.
   * This signal forwards to @code appearance_changed().
   */
  void tag_removed(omm::Object&, omm::Tag&);

  /**
   * @brief object_selection_changed is emitted when the object selection changed.
   */
  void object_selection_changed(const std::set<omm::Object*>&);

  /**
   * @brief style_selection_changed is emitted when the style selection changed.
   */
  void style_selection_changed(const std::set<omm::Style*>&);

  /**
   * @brief tag_selection_changed is emitted when the tag selection changed.
   */
  void tag_selection_changed(const std::set<omm::Tag*>&);

  /**
   * @brief node_selection_changed is emitted when the node selection changed.
   */
  void node_selection_changed(const std::set<omm::nodes::Node*>&);

  /**
   * @brief tool_selection_changed is emitted when the tool selection changed.
   */
  void tool_selection_changed(const std::set<omm::Tool*>&);

  /**
   * @brief selection_changed is emitted when the selection changed.
   */
  void selection_changed(const std::set<omm::AbstractPropertyOwner*>&);

  /**
   * @brief kind_selection_changed is emitted when tag, style, object or tool selection changed.
   * The kind of selection is transferred via the second argument.
   */
  void kind_selection_changed(const std::set<omm::AbstractPropertyOwner*>&, omm::Kind);

  /**
   * @brief filename_changed is emitted when the filename of the scene changes.
   * This includes changes of the pending-changes-indicator (usually an asterisk or nothing).
   */
  void filename_changed();

  /**
   * @brief point_selection_changed is emitted when points become (de)selected.
   * If you implement a feature that directly selects or deselects, i.e., alters the
   * `Point::is_selected` field, you should emit this signal to inform the scene.
   * You may defer the emission of the signal, e.g., if you select multiple points or if the
   * selection-modifications are tentative and become permanent later.
   * That's also the reason why it is required to emit this signal manually:
   * Often it's too early to emit it at the moment when the selection of a single point was changed.
   */
  void point_selection_changed();

  /**
   * @brief scene_reseted is emitted when the scene was reset.
   */
  void scene_reseted();

  /**
   * @brief appearance_changed is emitted when the style appearance changed.
   */
  void style_appearance_changed(omm::Style&);

  /**
   * @param owner the owner of the property
   * @param key the
   * @param property
   */

  /**
   * @brief property_value_changed is emitted when the value of the property has changed.
   * @param key the key of the property. The following invariant applies:
   *    `owner.property(key) == &p`
   */
  void
  property_value_changed(omm::AbstractPropertyOwner& owner, const QString& key, omm::Property& p);

  /**
   * @brief abstract_property_owner_inserted is emitted after any AbstractPropertyOwner is inserted.
   *  It is forwarded from object_inserted, style_inserted and tag_inserted.
   * @param owner the item that has been inserted.
   */
  void abstract_property_owner_inserted(omm::AbstractPropertyOwner& owner);

  /**
   * @brief abstract_property_owner_removed is emitted after any AbstractPropertyOwner is removed.
   *  It is forwarded from object_removed, style_remoced and tag_removed
   * @param property_owner the item that has been removed.
   * @note the ownership of the property_owner has been transferred before the signal is emitted.
   * I.e., if the new owner decides to delete the object, the reference is dangling.
   */
  void abstract_property_owner_removed(omm::AbstractPropertyOwner& property_owner);

  void about_to_reset();
};

}  // namespace omm
