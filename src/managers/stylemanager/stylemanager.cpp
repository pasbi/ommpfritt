#include "managers/stylemanager/stylemanager.h"
#include "managers/stylemanager/stylelistview.h"

namespace omm
{

StyleManager::StyleManager(Scene& scene)
  : Manager(tr("Styles"), scene)
  , m_style_list_adapter(scene)
{
  setWindowTitle(tr("style manager"));

  auto list_view = std::make_unique<StyleListView>(scene);
  list_view->set_model(&m_style_list_adapter);
  setWidget(list_view.release());
  setObjectName(TYPE());
}

StyleManager::~StyleManager()
{
}

}  // namespace omm
