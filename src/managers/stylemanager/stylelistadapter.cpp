#include "managers/stylemanager/stylelistadapter.h"
#include "renderers/style.h"

namespace omm
{

StyleListAdapter::StyleListAdapter(Scene& scene)
  : ItemModelAdapter(scene, scene.styles)
{
}

int StyleListAdapter::rowCount(const QModelIndex& parent) const
{
  assert(!parent.isValid());
  return structure().items().size();
}

QVariant StyleListAdapter::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  assert(!index.parent().isValid());

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return QString::fromStdString(structure().item(index.row()).name());
  }
  return QVariant();
}

Qt::ItemFlags StyleListAdapter::flags(const QModelIndex &index) const
{
  assert(!index.parent().isValid());
  return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren;
}

std::unique_ptr<AbstractRAIIGuard>
StyleListAdapter::acquire_inserter_guard(int row)
{
  class InserterGuard : public AbstractRAIIGuard
  {
  public:
    InserterGuard(StyleListAdapter& model, int row) : m_model(model)
    {
      m_model.beginInsertRows(QModelIndex(), row, row);
    }
    ~InserterGuard() { m_model.endInsertRows(); }
  private:
    StyleListAdapter& m_model;
  };
  return std::make_unique<InserterGuard>(*this, row);
}

  // friend class AbstractRAIIGuard;
  // std::unique_ptr<AbstractRAIIGuard> acquire_mover_guard() override;

std::unique_ptr<AbstractRAIIGuard>
StyleListAdapter::acquire_remover_guard(int row)
{
  class RemoverGuard : public AbstractRAIIGuard
  {
  public:
    RemoverGuard(StyleListAdapter& model, int row) : m_model(model)
    {
      m_model.beginRemoveRows(QModelIndex(), row, row);
    }
    ~RemoverGuard() { m_model.endRemoveRows(); }
  private:
    StyleListAdapter& m_model;
  };
  return std::make_unique<RemoverGuard>(*this, row);
}

std::unique_ptr<AbstractRAIIGuard>
StyleListAdapter::acquire_reseter_guard()
{
  class ReseterGuard : public AbstractRAIIGuard
  {
  public:
    ReseterGuard(StyleListAdapter& model) : m_model(model)
    {
      m_model.beginResetModel();
    }
    ~ReseterGuard() { m_model.endResetModel(); }
  private:
    StyleListAdapter& m_model;
  };
  return std::make_unique<ReseterGuard>(*this);
}

std::unique_ptr<AbstractRAIIGuard>
StyleListAdapter::acquire_mover_guard(const StyleListMoveContext& context)
{
  class MoverGuard : public AbstractRAIIGuard
  {
  public:
    MoverGuard(StyleListAdapter& model, const int old_pos, const int new_pos)
      : m_model(model)
    {
      LOG(INFO) << "beginMoveRows " << old_pos << " " << new_pos;
      m_model.beginMoveRows(QModelIndex(), old_pos, old_pos, QModelIndex(), new_pos);
    }

    ~MoverGuard() { m_model.endMoveRows(); }
  private:
    StyleListAdapter& m_model;
  };

  const auto old_pos = scene().styles.position(context.subject);
  const auto new_pos = ( context.predecessor == nullptr
                            ? 0 : scene().styles.position(*context.predecessor) + 1 );

  if (old_pos == new_pos) {
    return nullptr;
  } else {
    return std::make_unique<MoverGuard>(*this, old_pos, new_pos);
  }
}

bool StyleListAdapter::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole) {
    return false;
  }

  assert(index.column() == 0);
  assert(!index.parent().isValid());

  auto& style = structure().item(index.row());
  auto& name_property = style.property<StringProperty>(Object::NAME_PROPERTY_KEY);
  name_property.set_value(value.toString().toStdString());
  return true;
}

Style& StyleListAdapter::item_at(const QModelIndex& index) const
{
  assert(index.isValid());
  assert(!index.parent().isValid());
  return structure().item(index.row());
}

}  // namespace omm
