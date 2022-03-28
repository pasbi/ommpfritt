#pragma once

#include <functional>
#include <set>
#include <string>
#include <typeinfo>
#include <variant>

#include "abstractfactory.h"
#include "common.h"
#include "external/json.hpp"
#include "logging.h"
#include "propertyconfiguration.h"
#include "propertytypeenum.h"
#include "variant.h"

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Object;
class Track;
class Property;
class OptionProperty;

class Property : public QObject, public AbstractFactory<QString, true, Property>
{
  Q_OBJECT

public:
  explicit Property();
  explicit Property(const Property& other);
  ~Property() override;
  Property(Property&&) = delete;
  Property& operator=(Property&&) = delete;
  Property& operator=(const Property&) = delete;

  static constexpr auto LABEL_POINTER = "label";
  static constexpr auto CATEGORY_POINTER = "category";
  static constexpr auto ANIMATABLE_POINTER = "animatable";
  static constexpr auto TRACK_POINTER = "track";
  static constexpr auto IS_ANIMATED_POINTER = "animated";

  [[nodiscard]] QString widget_type() const;
  [[nodiscard]] Type data_type() const;
  static QString property_type(const Type data_type)
  {
    return QString::fromStdString(std::string{variant_type_name(data_type)}) + "Property";
  }

  template<typename ResultT, typename PropertyT, typename MemFunc>
  static ResultT get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    const auto values = util::transform(properties, [&f](const Property* property) {
      return f(static_cast<const PropertyT&>(*property));
    });

    if (values.size() > 1) {
      LWARNING << "expected uniform value, but got " << values.size() << " different values.";
    }

    return *values.begin();
  }

  template<typename ResultT, typename MemFunc>
  static ResultT get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    return Property::get_value<ResultT, Property, MemFunc>(properties, std::forward<MemFunc>(f));
  }

  [[nodiscard]] virtual bool is_compatible(const Property& other) const;
  static constexpr auto USER_PROPERTY_CATEGROY_NAME
      = QT_TRANSLATE_NOOP("Property", "user properties");

  // user properties can be added/edited/removed dynamically
  [[nodiscard]] bool is_user_property() const;

  // tracks of numeric properties can be displayed as fcurve.
  [[nodiscard]] virtual bool is_numerical() const = 0;

  virtual void revise();

  // === set/get value
public:
  [[nodiscard]] virtual variant_type variant_value() const = 0;

  /**
   * @brief set In gcc 9 and earlier, there is the issue that pointers will be converted to bool.
   * Hence, if you write `set("...")`, it will convert the string literal to a variant holding
   * a bool, although you most likely expected it to hold a QString.
   * Deleting his overload makes sure that one never falls into such a pitfall.
   * Call `set` with an explicitely constructed `variant_type` instead.
   */
  void set(const char*) = delete;
  virtual void set(const variant_type& value) = 0;
  template<typename EnumT> requires std::is_enum_v<EnumT> void set(const EnumT& value)
  {
    set(static_cast<std::size_t>(value));
  }

  template<typename ValueT> requires (!std::is_enum_v<ValueT>) ValueT value() const
  {
    return std::get<ValueT>(variant_value());
  }

  template<typename ValueT> requires std::is_enum_v<ValueT> ValueT value() const
  {
    return static_cast<ValueT>(std::get<std::size_t>(variant_value()));
  }

  // === Configuration ====
public:
  [[nodiscard]] bool is_visible() const;
  void set_visible(bool visible);
  [[nodiscard]] QString label() const;
  [[nodiscard]] QString category() const;
  [[nodiscard]] bool is_animatable() const;
  Property& set_label(const QString& label);
  Property& set_category(const QString& category);
  Property& set_animatable(bool animatable);
  PropertyConfiguration configuration;

private:
  bool m_is_visible = true;

  // === (De)Serialization
public:
  virtual void serialize(serialization::SerializerWorker& worker) const;
  virtual void deserialize(serialization::DeserializerWorker& worker);

  // === Animation
public:
  [[nodiscard]] Track* track() const;

  /**
   * @brief set_track sets the track, possibly overwriting the track set before (which is deleted).
   *  Thereafter, the property is animated unless the @code track is nullptr.
   *  the @code track's property must be @code this.
   * @param track the track.
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::insert_track
   */
  void set_track(std::unique_ptr<Track> track);

  /**
   * @brief extract_track extracts the track from this property.
   * Thereafter, the property is not animated anymore.
   * Possibly returns nullptr, if there was no track before set.
   * @return the track.
   * @note this function must only be called from the @code Animator instance.
   * @see Animator::extrack_track
   */
  std::unique_ptr<Track> extract_track();

private:
  std::unique_ptr<Track> m_track;

Q_SIGNALS:
  void value_changed(omm::Property*);

  /**
   * @brief configuration_changed is emitted when the configuration has changed.
   * @note property widgets should listen to this signal and update themselves accordingly.
   *  However, this feature is not implemented yet for most properties.
   *  It should be fairly simple to implement.
   *  Remove this note once the update mechanism is implemented in all PropertyWidgets.
   */
  void configuration_changed();

  void visibility_changed(bool);

public:
  // === Channels
  struct PropertyDetail {
    const std::function<QString(const Property&, std::size_t)> channel_name;
  };

  [[nodiscard]] std::size_t n_channels() const;
  [[nodiscard]] double channel_value(std::size_t channel) const;
  void set_channel_value(std::size_t channel, double value);
  [[nodiscard]] QString channel_name(std::size_t channel) const;

Q_SIGNALS:
  void enabledness_changed(bool);

public:
  void set_enabledness(bool enabled);

public:
  [[nodiscard]] bool is_enabled() const
  {
    return m_is_enabled;
  }

private:
  bool m_is_enabled = true;

public:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static std::map<QString, const PropertyDetail*> m_details;
};

}  // namespace omm
