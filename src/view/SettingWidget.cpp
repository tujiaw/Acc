#include "SettingWidget.h"
#include <QAction>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>
#include "controller/Acc.h"

SettingWidget::SettingWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pbHotkeyConfirm, &QPushButton::clicked, this, &SettingWidget::slotHotkeyConfirm);
	connect(ui.cbAutoStart, &QCheckBox::stateChanged, this, &SettingWidget::slotAutoStartChanged);

	ui.listWidget->addItem(tr("Hot Key"));

	SettingModel *settingModel = Acc::instance()->getSettingModel();
	ui.leHotkey->setText(settingModel->mainShortcutText());
	ui.cbAutoStart->setChecked(settingModel->autoStart());
}

SettingWidget::~SettingWidget()
{
}

void SettingWidget::slotHotkeyConfirm()
{
	QString text = ui.leHotkey->text().trimmed();
	text.remove(" ");
	emit Acc::instance()->sigSetMainShortcut(text);
}

void SettingWidget::slotAutoStartChanged(int state)
{
	Acc::instance()->getSettingModel()->setAutoStart(state == Qt::Checked);
}

void SettingWidget::keyPressEvent(QKeyEvent *e)
{
	QKeySequence key(e->key());
	QWidget::keyPressEvent(e);
}
