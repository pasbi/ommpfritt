#pragma once

#include <QMenu>

namespace omm
{

class Scene;

class PathMenu : public QMenu
{
public:
  PathMenu(Scene& scene, QWidget* parent);
};

}  // namespace omm
