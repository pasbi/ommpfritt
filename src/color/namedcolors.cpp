#include "color/namedcolors.h"
#include "logging.h"
#include "serializers/serializerworker.h"
#include "serializers/deserializerworker.h"

namespace omm
{

int NamedColors::rowCount(const QModelIndex& parent) const
{
  assert(!parent.isValid());
  Q_UNUSED(parent)
  return static_cast<int>(m_named_colors.size());
}

QVariant NamedColors::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return {};
  }
  const auto& [name, color] = m_named_colors[index.row()];
  switch (role) {
  case Qt::BackgroundRole:
    return color.to_qcolor();
  case Qt::DisplayRole:
    return name;
  default:
    return {};
  }
}

Color* NamedColors::resolve(const QString& name)
{
  const auto it = std::find_if(m_named_colors.begin(),
                               m_named_colors.end(),
                               [name](const auto& pair) { return pair.first == name; });
  if (it == m_named_colors.end()) {
    return nullptr;
  } else {
    assert(it->second.model() != Color::Model::Named);
    return &it->second;
  }
}

const Color* NamedColors::resolve(const QString& name) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<const Color*>(const_cast<NamedColors*>(this)->resolve(name));
}

bool NamedColors::resolve(const QString& name, Color& color) const
{
  const Color* c = resolve(name);
  if (c != nullptr) {
    color = *c;
    return true;
  } else {
    return false;
  }
}

Color NamedColors::color(const QModelIndex& index) const
{
  assert(index.isValid());
  assert(index.model() == this);
  return m_named_colors[index.row()].second;
}

QString NamedColors::name(const QModelIndex& index) const
{
  assert(index.isValid());
  assert(index.model() == this);
  return m_named_colors[index.row()].first;
}

QModelIndex NamedColors::index(const QString& name) const
{
  const auto it = std::find_if(m_named_colors.begin(),
                               m_named_colors.end(),
                               [name](const auto& pair) { return pair.first == name; });
  return index(static_cast<int>(std::distance(m_named_colors.begin(), it)), 0);
}

bool NamedColors::has_color(const QString& name) const
{
  const auto it = std::find_if(m_named_colors.begin(),
                               m_named_colors.end(),
                               [name](const auto& pair) { return pair.first == name; });
  return it != m_named_colors.end();
}

Qt::ItemFlags NamedColors::flags(const QModelIndex& index) const
{
  Q_UNUSED(index)
  return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void NamedColors::set_color(const QModelIndex& index, const Color& color)
{
  assert(index.isValid() && index.model() == this);
  m_named_colors[index.row()].second = color;
  Q_EMIT dataChanged(index, index, {Qt::BackgroundRole, Qt::ForegroundRole});
}

void NamedColors::change(const QString& name, const Color& color)
{
  auto& pair = m_named_colors[index(name).row()];
  assert(color.model() != Color::Model::Named);
  pair.second = color;
}

void NamedColors::rename(const QString& old_name, const QString& new_name)
{
  assert(!has_color(new_name));
  assert(has_color(old_name));
  const QModelIndex index = this->index(old_name);
  auto& pair = m_named_colors[index.row()];
  Q_EMIT dataChanged(index, index, {Qt::DisplayRole});
  pair.first = new_name;
}

void NamedColors::remove(const QString& name)
{
  const QModelIndex index = this->index(name);
  assert(index.isValid());
  assert(index.model() == this);
  const auto n = static_cast<int>(index.row());
  beginRemoveRows({}, n, n);
  m_named_colors.erase(m_named_colors.begin() + n);
  endRemoveRows();
}

Color NamedColors::color(const QString& name) const
{
  const QModelIndex index = this->index(name);
  assert(index.isValid());
  assert(index.model() == this);
  return m_named_colors[index.row()].second;
}

void NamedColors::clear()
{
  beginResetModel();
  m_named_colors.clear();
  endResetModel();
}

void NamedColors::serialize(serialization::SerializerWorker& worker) const
{
  worker.set_value(m_named_colors, [](const auto& named_color, auto& worker_i) {
    const auto& [name, color] = named_color;
    worker_i.sub("name")->set_value(name);
    worker_i.sub("color")->set_value(color);
  });
}

void NamedColors::deserialize(serialization::DeserializerWorker& worker)
{
  beginResetModel();
  m_named_colors.clear();
  worker.get_items([this](auto& worker_i) {
    const auto name = worker_i.sub("name")->get_string();
    const auto color = worker_i.sub("color")->template get<Color>();
    m_named_colors.emplace_back(name, color);
  });
  endResetModel();
}

QString NamedColors::generate_default_name() const
{
  const QString default_name = tr("Unnamed Color");
  QString candidate = default_name;
  std::size_t i = 0;
  while (has_color(candidate)) {
    i += 1;
    static const int base = 10;
    static const int width = 3;
    candidate = default_name + QString(".%1").arg(static_cast<int>(i), width, base, QChar('0'));
  }
  return candidate;
}

QModelIndex NamedColors::add(const QString& name, const Color& color)
{
  const auto n = static_cast<int>(m_named_colors.size());
  beginInsertRows(QModelIndex(), n, n);
  m_named_colors.emplace_back(name, color);
  endInsertRows();
  return index(n, 0, QModelIndex());
}

}  // namespace omm
