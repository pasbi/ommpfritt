#include "objects/abstractproceduralpath.h"
#include "properties/propertygroups/markerproperties.h"

namespace omm
{

class Tip : public AbstractProceduralPath
{
public:
  explicit Tip(Scene *scene);
  Tip(const Tip& other);
  std::vector<Point> points() const override;
  QString type() const override;
  bool is_closed() const override;
  void on_property_value_changed(Property* property) override;

private:
  MarkerProperties m_marker_properties;
};

}  // namespace omm
