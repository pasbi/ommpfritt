#include "keybindingsdialogcontrols.h"

#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QComboBox>
#include <memory>
#include "keybindingsproxymodel.h"
#include "common.h"
#include <QtGlobal>

namespace omm
{

KeyBindingsDialogControls::KeyBindingsDialogControls(KeyBindingsProxyModel &filter_proxy)
  : m_filter_proxy(filter_proxy)
{
  auto layout = std::make_unique<QHBoxLayout>();

  auto filter_line_edit = std::make_unique<QLineEdit>();
  m_filter_line_edit = filter_line_edit.get();
  connect(m_filter_line_edit, &QLineEdit::textChanged, [this](const QString& text) {
    m_filter_proxy.set_action_filter(text.toStdString());
  });
  layout->addWidget(filter_line_edit.release());

  auto context_filter_combobox = std::make_unique<QComboBox>();
  m_context_filter_combobox = context_filter_combobox.get();
  const auto contextes = ::transform<QString, QList>(m_filter_proxy.contextes(), [](auto&& s) {
    return QString::fromStdString(s);
  });
  m_context_filter_combobox->addItems(contextes);
  m_context_filter_combobox->insertItem(0, tr("Any Context"));
  m_context_filter_combobox->setCurrentIndex(0);
  connect( m_context_filter_combobox,
           static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           [this](int index) {
    if (index == 0) {
      m_filter_proxy.set_context_filter("");
    } else {
      m_filter_proxy.set_context_filter(m_context_filter_combobox->itemText(index).toStdString());
    }
  });
  layout->addWidget(context_filter_combobox.release());

  auto clear_button = std::make_unique<QPushButton>();
  m_clear_button = clear_button.get();
  layout->addWidget(clear_button.release());
  connect(m_clear_button, &QPushButton::clicked, [this]() {
    m_context_filter_combobox->setCurrentIndex(0);
    m_filter_line_edit->setText("");
  });

  setLayout(layout.release());
}

}  // namespace omm
