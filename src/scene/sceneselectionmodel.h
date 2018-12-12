#pragma once

#include <QItemSelectionModel>

namespace omm
{

class SceneSelectionModel : public QItemSelectionModel
{

  

private:
  using QItemSelectionModel::setModel;

};

}  // namespace omm