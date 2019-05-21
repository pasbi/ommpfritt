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
  std::string type() const override;
  std::unique_ptr<Object> clone() const override;
  bool is_closed() const override;

private:
  MarkerProperties m_marker_properties;
};

}  // namespace omm
