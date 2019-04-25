#include "keybindings/keybindingsdialog.h"
#include <QTableView>
#include <QDialogButtonBox>
#include <memory>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QHeaderView>
#include "keybindings/keybindings.h"
#include "keybindings/keybindingstable.h"
#include "keybindings/keybindingsdialogcontrols.h"
#include <QSettings>

namespace omm
{

KeyBindingsDialog::KeyBindingsDialog(KeyBindings& key_bindings, QWidget* parent)
  : QDialog(parent)
  , m_proxy_model(KeyBindingsProxyModel(key_bindings))
{
  auto layout = std::make_unique<QVBoxLayout>();

  auto controls = std::make_unique<KeyBindingsDialogControls>(m_proxy_model);

  auto edit = std::make_unique<KeyBindingsTable>(m_proxy_model);
  edit->horizontalHeader()->setStretchLastSection(true);

  layout->addWidget(controls.release());
  layout->addWidget(edit.release());

  const auto buttons = QDialogButtonBox::Ok;
  auto button_box = std::make_unique<QDialogButtonBox>(buttons);
  connect( button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked,
           this, &KeyBindingsDialog::accept );
  layout->addWidget(button_box.release());

  setLayout(layout.release());

  QSettings settings;
  settings.beginGroup(KEYBINDINGS_DIALOG_SETTINGS_GROUP);
  restoreGeometry(settings.value(GEOMETRY_SETTINGS_KEY).toByteArray());
  settings.endGroup();
}

KeyBindingsDialog::~KeyBindingsDialog()
{
  QSettings settings;
  settings.beginGroup(KEYBINDINGS_DIALOG_SETTINGS_GROUP);
  settings.setValue(GEOMETRY_SETTINGS_KEY, saveGeometry());
  settings.endGroup();
}

}  // namespace omm
