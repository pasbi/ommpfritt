#pragma once

#include <functional>
#include <string>
#include <typeinfo>
#include <set>
#include <variant>
#include <functional>

#include "logging.h"
#include "external/json.hpp"
#include "abstractfactory.h"
#include "aspects/serializable.h"
#include "common.h"
#include "color/color.h"
#include "geometry/vec2.h"
#include "variant.h"
#include "animation/track.h"
#include "dnf.h"

namespace omm
{

class Object;
class Track;
class Property;
class OptionProperty;

class Property
  : public QObject
  , public AbstractFactory<QString, true, Property>
  , public virtual Serializable
{
  Q_OBJECT


public:
  struct Filter : public Serializable {
    explicit Filter(const Disjunction<Kind>& kind,
                    const DNF<Flag>& flag);
    Filter();
    explicit Filter(const DNF<Flag>& flag);
    void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
    void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
    Disjunction<Kind> kind;
    DNF<Flag> flag;
    bool accepts(const AbstractPropertyOwner& apo) const;
    bool accepts(Kind kind, Flag flag) const;
    bool operator==(const Filter& other) const;
    bool operator!=(const Filter& other) const { return !(*this == other); }
    bool operator<(const Filter& other) const;

    static Filter accept_anything();
  };

  struct Configuration : std::map<QString, std::variant<bool, int, double, Vec2i, Vec2f,
                                                        std::size_t, QString,
                                                        std::vector<QString>,
                                                        Filter>>
  {
    using variant_type = value_type::second_type;
    template<typename T> T get(const QString& key) const
    {
      if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(get<std::size_t>(key));
      } else {
        const auto cit = find(key);
        assert (cit != end());

        const T* value = std::get_if<T>(&cit->second);
        assert(value != nullptr);
        return *value;
      }
    }

    template<typename T> T get(const QString& key, const T& default_value) const
    {
      if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(get<std::size_t>(key, default_value));
      } else {
        const auto cit = find(key);
        if (cit == end()) {
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

    template<typename T>
    void deserialize_field(const QString& field, AbstractDeserializer& deserializer,
                           const Serializable::Pointer& root)
    {
      if (this->find(field) == this->end()) {
        (*this)[field] = deserializer.get<T>(Serializable::make_pointer(root, field));
      }
    }

  };

  Property() = default;
  explicit Property(const Property& other);
  virtual ~Property() = default;

  static constexpr auto LABEL_POINTER = "label";
  static constexpr auto CATEGORY_POINTER = "category";
  static constexpr auto ANIMATABLE_POINTER = "animatable";
  static constexpr auto TRACK_POINTER = "track";
  static constexpr auto IS_ANIMATED_POINTER = "animated";

  QString widget_type() const;
  QString data_type() const;

  template<typename ResultT, typename PropertyT, typename MemFunc> static
  ResultT get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    const auto values = ::transform<ResultT>(properties, [&f](const Property* property) {
      return f(static_cast<const PropertyT&>(*property));
    });

    if (values.size() > 1) {
      LWARNING << "expected uniform value, but got " << values.size() << " different values.";
    }

    return *values.begin();
  }

  template<typename ResultT, typename MemFunc> static ResultT
  get_value(const std::set<Property*>& properties, MemFunc&& f)
  {
    return Property::get_value<ResultT, Property, MemFunc>(properties, std::forward<MemFunc>(f));
  }

  virtual bool is_compatible(const Property& other) const;
  static constexpr auto USER_PROPERTY_CATEGROY_NAME = QT_TRANSLATE_NOOP("Property",
                                                                        "user properties");

  // user properties can be added/edited/removed dynamically
  bool is_user_property() const;

  // tracks of numeric properties can be displayed as fcurve.
  virtual bool is_numerical() const = 0;

  virtual void revise();

  // === set/get value
public:
  virtual variant_type variant_value() const = 0;
  virtual void set(const variant_type& value) = 0;
  template<typename EnumT> std::enable_if_t<std::is_enum_v<EnumT>, void>
  set(const EnumT& value) { set(static_cast<std::size_t>(value)); }
  template<typename ValueT> std::enable_if_t<!std::is_enum_v<ValueT>, ValueT>
  value() const { return std::get<ValueT>(variant_value()); }
  template<typename ValueT> std::enable_if_t<std::is_enum_v<ValueT>, ValueT>
  value() const { return static_cast<ValueT>(std::get<std::size_t>(variant_value())); }

  // === Configuration ====
public:
  bool is_visible() const;
  void set_visible(bool visible);
  QString label() const;
  QString category() const;
  bool is_animatable() const;
  Property& set_label(const QString& label);
  Property& set_category(const QString& category);
  Property& set_animatable(bool animatable);
  Configuration configuration;
private:
  bool m_is_visible = true;

  // === (De)Serialization
public:
  void serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root) override;

  // === Animation
public:
  Track* track() const;

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
  void value_changed(Property*);

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
  struct PropertyDetail
  {
    const std::function<QString(const Property&, std::size_t)> channel_name;
  };

  std::size_t n_channels() const;
  double channel_value(std::size_t channel) const;
  void set_channel_value(std::size_t channel, double value);
  QString channel_name(std::size_t channel) const;

Q_SIGNALS:
  void enabledness_changed(bool);
public Q_SLOTS:
  void set_enabledness(bool enabled);
public:
  bool is_enabled() const { return m_is_enabled; }
private:
  bool m_is_enabled = true;

public:
  static std::map<QString, const PropertyDetail*> m_details;
};

std::ostream& operator<<(std::ostream& ostream, const Property::Filter& filter);

}  // namespace omm
