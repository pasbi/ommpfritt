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
#include <glog/logging.h>

namespace omm
{

KeyBindingsDialog::KeyBindingsDialog(KeyBindings& key_bindings, QWidget* parent) : QDialog(parent)
{
  auto layout = std::make_unique<QVBoxLayout>();

  auto edit = std::make_unique<KeyBindingsTable>(key_bindings);
  edit->horizontalHeader()->setStretchLastSection(true);
  QTimer::singleShot(1000, [this, edit=edit.get()]() {
    LOG(INFO) << edit->sizeHint().width();
    LOG(INFO) << "adjust"; edit->adjustSize(); this->adjustSize();
  });
  layout->addWidget(edit.release());

  const auto buttons = QDialogButtonBox::Ok;
  auto button_box = std::make_unique<QDialogButtonBox>(buttons);
  connect( button_box->button(QDialogButtonBox::Ok), &QPushButton::clicked,
           this, &KeyBindingsDialog::accept );
  layout->addWidget(button_box.release());

  setLayout(layout.release());
}

}  // namespace omm