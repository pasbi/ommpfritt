#include "managers/boundingboxmanager/boundingboxmanager.h"

#include "managers/boundingboxmanager/anchorwidget.h"

namespace omm
{

BoundingBoxManager::BoundingBoxManager(Scene& scene)
  : Manager(tr("Bounding Box Manager"), scene)
{
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  auto layout = std::make_unique<QHBoxLayout>();
  auto anchor_widget = std::make_unique<AnchorWidget>();
  layout->addWidget(anchor_widget.release());
  widget->setLayout(layout.release());

  set_widget(std::move(widget));
}

std::string BoundingBoxManager::type() const { return TYPE;  }

}  // namespace omm
