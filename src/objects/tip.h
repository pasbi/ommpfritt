#include "objects/abstractproceduralpath.h"

namespace omm
{

class Tip : public AbstractProceduralPath
{
public:
  explicit Tip(Scene *scene);
  std::vector<Point> points() const override;
  std::string type() const override;
  std::unique_ptr<Object> clone() const override;
  bool is_closed() const override;

  enum class Shape { None, Arrow, Bar, Circle, Diamond };
  static std::vector<std::string> shapes();
  static constexpr auto SHAPE_KEY = "shape";

  static constexpr auto BASE_SIZE_KEY = "base";
  static constexpr auto ASPECT_RATIO_KEY = "aspect_ratio";

  static std::vector<Point> shape(Shape shape, const double size, const double aspect_ratio);
  static std::vector<Point> arrow(const Vec2f& size);
  static std::vector<Point> bar(const Vec2f& size);
  static std::vector<Point> circle(const Vec2f& size);
  static std::vector<Point> diamond(const Vec2f& size);
};

}  // namespace omm
