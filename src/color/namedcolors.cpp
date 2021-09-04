#include "color/namedcolors.h"
#include "logging.h"
#include "serializers/abstractserializer.h"

namespace omm
{
int NamedColors::rowCount(const QModelIndex& parent) const
{
  assert(!parent.isValid());
  Q_UNUSED(parent)
  return m_named_colors.size();
}

QVariant NamedColors::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }
  const auto& [name, color] = m_named_colors[index.row()];
  switch (role) {
  case Qt::BackgroundRole:
    return color.to_qcolor();
  case Qt::DisplayRole:
    return name;
  default:
    return QVariant();
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
  return index(std::distance(m_named_colors.begin(), it), 0);
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
  const std::size_t n = index.row();
  beginRemoveRows(QModelIndex(), n, n);
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

void NamedColors::serialize(AbstractSerializer& serializer, const Serializable::Pointer& p) const
{
  serializer.start_array(m_named_colors.size(), Serializable::make_pointer(p));
  for (std::size_t i = 0; i < m_named_colors.size(); ++i) {
    const auto& [name, color] = m_named_colors[i];
    serializer.set_value(name, Serializable::make_pointer(p, i, "name"));
    serializer.set_value(color, Serializable::make_pointer(p, i, "color"));
  }
  serializer.end_array();
}

void NamedColors::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& p)
{
  const std::size_t n = deserializer.array_size(make_pointer(p));
  beginResetModel();
  m_named_colors.clear();
  m_named_colors.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    const auto name = deserializer.get_string(make_pointer(p, i, "name"));
    const auto color = deserializer.get_color(make_pointer(p, i, "color"));
    m_named_colors.emplace_back(name, color);
  }
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
  const std::size_t n = m_named_colors.size();
  beginInsertRows(QModelIndex(), n, n);
  m_named_colors.emplace_back(name, color);
  endInsertRows();
  return index(n, 0, QModelIndex());
}

}  // namespace omm
