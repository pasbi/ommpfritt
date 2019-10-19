#pragma once

#include <QComboBox>
#include <QAbstractItemView>
#include <QIdentityProxyModel>
#include <QAbstractListModel>
#include "color/color.h"
#include <cmath>
#include "aspects/serializable.h"

namespace omm
{

class NamedColors : public QAbstractListModel, public Serializable
{
  Q_OBJECT
public:
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& data, int role) override;
  bool resolve(const std::string& name, Color& color) const;
  Color color(const QModelIndex& index) const;
  bool has_color(const std::string& name) const;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  void set_color(const QModelIndex& index, const Color& color);

  void serialize(AbstractSerializer&serializer, const Pointer&p) const override;
  void deserialize(AbstractDeserializer&deserializer, const Pointer&p) override;

public Q_SLOTS:
  QModelIndex add(const Color& color);
  void remove(const QModelIndex& index);

private:
  Color* resolve(const std::string& name);
  const Color* resolve(const std::string& name) const;
  std::vector<std::pair<std::string, Color>> m_named_colors;
};

template<typename ViewT>
class NamedColorsHighlighProxyModel : public QIdentityProxyModel
{
  static constexpr bool is_combobox = std::is_base_of_v<QComboBox, ViewT>;
  static constexpr bool is_itemview = std::is_base_of_v<QAbstractItemView, ViewT>;
public:
  explicit NamedColorsHighlighProxyModel(NamedColors& model, const ViewT& view)
    : m_view(view)
  {
    setSourceModel(&model);
    if constexpr (std::is_base_of_v<QComboBox, ViewT>) {
      connect(&view, qOverload<int>(&QComboBox::highlighted), [this](int index) {
        m_highlighted_index = index;
      });
    }
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override
  {
    if (!index.isValid()) {
      return 0;
    }
    if constexpr (is_combobox) {
      return mapToSource(index).flags() | Qt::ItemIsSelectable;
    } else {
      return mapToSource(index).flags();
    }
  }

  QVariant data(const QModelIndex& index, int role) const override
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
      QColor bg = data.template value<QColor>();
      if (is_highlighted) {
        bg = highlight_color(bg);
        if (role == Qt::ForegroundRole) {
          return get_contrast_color(bg, true);
        } else {
          return bg;
        }
      } else {
        if (role == Qt::ForegroundRole) {
          return get_contrast_color(bg, false);
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
      h = std::fmod(h + 0.5, 1.0);
      v = v < 0.75 ? 1.0 : 5.0;
      s = 1.0;
    } else {
      v = v < 0.5 ? 1.0 : 0.0;
      s = 0.0;
    }
    return Color(Color::Model::HSVA, hsva).to_qcolor();
  }

  static QColor highlight_color(const QColor& color)
  {
    auto hsva = Color::from_qcolor(color).components(Color::Model::HSVA);
    double& v = hsva[2];
    if (v > 0.5) {
      v -= 0.2;
    } else {
      v += 0.2;
    }
    return Color(Color::Model::HSVA, hsva).to_qcolor();
  }
};

}  // namespace omm
