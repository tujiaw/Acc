#include "SettingWidget.h"
#include <QAction>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>
#include "controller/Acc.h"
#include "view/MainWidget.h"
#include "common/DarkStyle.h"

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
	connect(ui.hsOpacity, &QSlider::sliderReleased, this, &SettingWidget::slotOpacityChanged);
	connect(ui.fcbFont, &QFontComboBox::currentFontChanged, this, &SettingWidget::slotCurrentFontChanged);

	QStringList menuList = QStringList() << tr("Hot Key") << tr("Start") << tr("Shown");
	for (int i = 0; i < menuList.size(); i++) {
		ui.listWidget->addItem(menuList[i]);
	}
	ui.listWidget->setCurrentRow(0);

	SettingModel *settingModel = Acc::instance()->getSettingModel();
	ui.leHotkey->setText(settingModel->mainShortcutText());
	ui.cbAutoStart->setChecked(settingModel->autoStart());

	ui.hsOpacity->setValue(ui.hsOpacity->maximum() - settingModel->mainOpacity());
	ui.fcbFont->setCurrentText(Acc::instance()->getSettingModel()->fontFamily());
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

void SettingWidget::slotOpacityChanged()
{
	int val = ui.hsOpacity->value();
	Acc::instance()->setWindowOpacity(WidgetID::MAIN, ui.hsOpacity->maximum() - val);
	Acc::instance()->getSettingModel()->setMainOpacity(ui.hsOpacity->maximum() - val);
}

void SettingWidget::slotCurrentFontChanged(const QFont &font)
{
	CDarkStyle::setFontFamily(font.family());
	CDarkStyle::assign();
	Acc::instance()->getSettingModel()->setFontFamily(font.family());
}