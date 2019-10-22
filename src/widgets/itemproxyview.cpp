#include "widgets/itemproxyview.h"

namespace omm
{

void LinkItemSelectionModel::setCurrentIndex(const QModelIndex&, QItemSelectionModel::SelectionFlags)
{
  // I don't know why, but the base implementation crashes.
}

}
