#pragma once

#include <set>
#include <armadillo>

#include "geometry/objecttransformation.h"

namespace omm
{

class Object;
class AbstractRenderer;

class Handle
{
public:
  explicit Handle(const std::set<Object*>& selection);
  Handle();
  /**
   * @brief returns true if any grabbable was moved
   */
  bool mouse_move(const arma::vec2& delta);

  /**
   * @brief returns true if any grabbable was hit
   */
  bool mouse_press(const arma::vec2& pos);

  void mouse_release();
  void draw(AbstractRenderer& renderer) const;

protected:
  virtual ObjectTransformation transformation() const = 0;
  const std::set<Object*>& objects() const;

private:
  std::set<Object*> m_objects;
};

class GlobalOrientedHandle : public Handle
{
protected:
  using Handle::Handle;
  ObjectTransformation transformation() const override;
};

}  // namespace omm
