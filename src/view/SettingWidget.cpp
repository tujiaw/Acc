#include "SettingWidget.h"
#include <QAction>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>
#include "controller/Acc.h"
#include "view/MainWidget.h"

SettingWidget::SettingWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	for (int i = 3; i < 10; i++) {
		ui.cbMaxResult->addItem(QString::number(i));
	}
	ui.cbMaxResult->setCurrentText(QString::number(Acc::instance()->getSettingModel()->maxResult()));
	connect(ui.cbMaxResult, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotMaxResultChanged(QString)));
	connect(ui.pbHotkeyConfirm, &QPushButton::clicked, this, &SettingWidget::slotHotkeyConfirm);
	connect(ui.cbAutoStart, &QCheckBox::stateChanged, this, &SettingWidget::slotAutoStartChanged);

	ui.listWidget->addItem(tr("Hot Key"));
	ui.listWidget->addItem(tr("Start"));
	ui.listWidget->addItem(tr("Shown"));
	ui.listWidget->setCurrentRow(0);

	SettingModel *settingModel = Acc::instance()->getSettingModel();
	ui.leHotkey->setText(settingModel->mainShortcutText());
	ui.cbAutoStart->setChecked(settingModel->autoStart());
}

SettingWidget::~SettingWidget()
{
}

void SettingWidget::slotMaxResultChanged(const QString &text)
{
	bool ok;
	int count = text.toInt(&ok);
	if (ok) {
		Acc::instance()->getSettingModel()->setMaxResult(count);
		emit Acc::instance()->sigClearResult();
	}
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
