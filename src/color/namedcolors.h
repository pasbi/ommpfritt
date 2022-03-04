#pragma once

#include "color/color.h"
#include <QAbstractItemView>
#include <QAbstractListModel>
#include <QComboBox>
#include <QIdentityProxyModel>
#include <cmath>
#include <deque>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class NamedColors : public QAbstractListModel
{
  Q_OBJECT
public:
  [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  bool resolve(const QString& name, Color& color) const;
  [[nodiscard]] Color color(const QModelIndex& index) const;
  [[nodiscard]] QString name(const QModelIndex& index) const;
  using QAbstractListModel::index;
  [[nodiscard]] QModelIndex index(const QString& name) const;
  [[nodiscard]] bool has_color(const QString& name) const;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
  void set_color(const QModelIndex& index, const Color& color);

  void change(const QString& name, const Color& color);
  void rename(const QString& old_name, const QString& new_name);
  QModelIndex add(const QString& name, const Color& color);
  void remove(const QString& name);
  [[nodiscard]] Color color(const QString& name) const;
  void clear();

  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  [[nodiscard]] QString generate_default_name() const;

private:
  Color* resolve(const QString& name);
  [[nodiscard]] const Color* resolve(const QString& name) const;
  std::deque<std::pair<QString, Color>> m_named_colors;
};

template<typename ViewT> class NamedColorsHighlighProxyModel : public QIdentityProxyModel
{
  static constexpr bool is_combobox = std::is_base_of_v<QComboBox, ViewT>;
  static constexpr bool is_itemview = std::is_base_of_v<QAbstractItemView, ViewT>;

public:
  explicit NamedColorsHighlighProxyModel(NamedColors& model, const ViewT& view) : m_view(view)
  {
    setSourceModel(&model);
    if constexpr (std::is_base_of_v<QComboBox, ViewT>) {
      connect(&view, qOverload<int>(&QComboBox::highlighted), [this](int index) {
        m_highlighted_index = index;
      });
    }
  }

  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    if (!index.isValid()) {
      return {};
    }
    if constexpr (is_combobox) {
      return mapToSource(index).flags() | Qt::ItemIsSelectable;
    } else {
      return mapToSource(index).flags();
    }
  }

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override
  {
    bool is_highlighted = false;
    if constexpr (is_itemview) {
      is_highlighted = index == m_view.currentIndex();
    } else if constexpr (is_combobox) {
      is_highlighted = index.row() == m_highlighted_index;
    } else {
      // static_assert(false) is not possible, see
      // https://stackoverflow.com/questions/38304847/constexpr-if-and-static-assert
      static_assert(std::is_same_v<ViewT, void>,
                    "ViewT must inherit QComboBox or QAbstractItemView");
    }

    if (role == Qt::BackgroundRole || role == Qt::ForegroundRole) {
      const QVariant data = sourceModel()->data(mapToSource(index), Qt::BackgroundRole);
      if (is_highlighted) {
        if (role == Qt::ForegroundRole) {
          return QColor(Qt::blue);
        } else {
          return QColor(Qt::white);
        }
      } else {
        auto bg = data.template value<QColor>();
        if (role == Qt::ForegroundRole) {
          static constexpr double VALUE_THRESHOLD = 0.5;
          if (bg.valueF() < VALUE_THRESHOLD) {
            return QColor(Qt::white);
          } else {
            return QColor(Qt::black);
          }
        } else {
          return bg;
        }
      }
    } else {
      return sourceModel()->data(mapToSource(index), role);
    }
  }

private:
  const ViewT& m_view;
  int m_highlighted_index = -1;

  static QColor get_contrast_color(const QColor& color, bool highlight)
  {
    auto hsva = Color::from_qcolor(color).components(Color::Model::HSVA);
    double& v = hsva[2];
    double& s = hsva[1];
    double& h = hsva[0];
    hsva[3] = 1.0;
    if (highlight) {
      static constexpr double VALUE_THRESHOLD = 0.75;
      static constexpr double HALF_HUE_RANGE = 0.5;
      h = std::fmod(h + HALF_HUE_RANGE, 1.0);
      v = v < VALUE_THRESHOLD ? 1.0 : 0.0;
      s = 1.0;
    } else {
      static constexpr double VALUE_THRESHOLD = 0.5;
      v = v < VALUE_THRESHOLD ? 1.0 : 0.0;
      s = 0.0;
    }
    return Color(Color::Model::HSVA, hsva).to_qcolor();
  }

  static QColor highlight_color(const QColor& color)
  {
    auto hsva = Color::from_qcolor(color).components(Color::Model::HSVA);
    double& v = hsva[2];
    static constexpr double VALUE_THRESHOLD = 0.5;
    static constexpr double VALUE_OFFSET = 0.2;
    if (v > VALUE_THRESHOLD) {
      v -= VALUE_OFFSET;
    } else {
      v += VALUE_OFFSET;
    }
    return Color(Color::Model::HSVA, hsva).to_qcolor();
  }
};

}  // namespace omm
