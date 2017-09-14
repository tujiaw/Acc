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
	
	connect(ui.cbMaxResult, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotMaxResultChanged(QString)));
	connect(ui.pbHotkeyConfirm, &QPushButton::clicked, this, &SettingWidget::slotHotkeyConfirm);
	connect(ui.cbAutoStart, &QCheckBox::stateChanged, this, &SettingWidget::slotAutoStartChanged);
	connect(ui.hsOpacity, &QSlider::sliderReleased, this, &SettingWidget::slotOpacityChanged);
	connect(ui.fcbFont, &QFontComboBox::currentFontChanged, this, &SettingWidget::slotCurrentFontChanged);
	connect(ui.cbBold, &QCheckBox::stateChanged, this, &SettingWidget::slotBoldChanged);
	connect(ui.labelDefault, &QLabel::linkActivated, this, &SettingWidget::slotDefaultActivated);

	QStringList menuList = QStringList() << tr("Hot Key") << tr("Start") << tr("Shown");
	for (int i = 0; i < menuList.size(); i++) {
		ui.listWidget->addItem(menuList[i]);
	}

	readData();
}

SettingWidget::~SettingWidget()
{
}

void SettingWidget::readData()
{
	ui.listWidget->setCurrentRow(0);
	SettingModel *settingModel = Acc::instance()->getSettingModel();
	ui.cbMaxResult->setCurrentText(QString::number(settingModel->maxResult()));
	ui.leHotkey->setText(settingModel->mainShortcutText());
	ui.cbAutoStart->setChecked(settingModel->autoStart());
	ui.hsOpacity->setValue(ui.hsOpacity->maximum() - settingModel->mainOpacity());
	ui.fcbFont->setCurrentText(Acc::instance()->getSettingModel()->fontFamily());
	ui.cbBold->setChecked(Acc::instance()->getSettingModel()->isBold());
}

void SettingWidget::writeData(QObject *sender)
{
	SettingModel *settingModel = Acc::instance()->getSettingModel();

	// 最大显示列个数
	if (!sender || ui.cbMaxResult == sender) {
		bool ok;
		int count = ui.cbMaxResult->currentText().toInt(&ok);
		if (ok && settingModel->maxResult() != count) {
			settingModel->setMaxResult(count);
			emit Acc::instance()->sigClearResult();
		}
	}

	// 热键
	if (!sender || ui.leHotkey == sender) {
		QString text = ui.leHotkey->text().trimmed();
		text.remove(" ");
		if (settingModel->mainShortcutText() != text) {
			emit Acc::instance()->sigSetMainShortcut(text);
		}
	}
	
	// 开机重启
	if (!sender || ui.cbAutoStart == sender) {
		bool isAutoStart = ui.cbAutoStart->isChecked();
		if (isAutoStart != settingModel->autoStart()) {
			Acc::instance()->getSettingModel()->setAutoStart(isAutoStart);
		}
	}

	// 透明度
	if (!sender || ui.hsOpacity == sender) {
		int opacity = ui.hsOpacity->maximum() - ui.hsOpacity->value();
		if (settingModel->mainOpacity() != opacity) {
			Acc::instance()->setWindowOpacity(WidgetID::MAIN, opacity);
			Acc::instance()->getSettingModel()->setMainOpacity(opacity);
		}
	}

	// 字体
	if (!sender || (ui.cbBold == sender || ui.fcbFont == sender)) {
		bool isBold = ui.cbBold->isChecked();
		QString family = ui.fcbFont->currentText();
		if (isBold != settingModel->isBold() || family != settingModel->fontFamily()) {
			CDarkStyle::setFontFamily(family, isBold);
			CDarkStyle::assign();
			Acc::instance()->getSettingModel()->setFontFamily(family, isBold);
		}
	}
}

void SettingWidget::slotMaxResultChanged(const QString &text)
{
	writeData(sender());
}

void SettingWidget::slotHotkeyConfirm()
{
	writeData(sender());
}

void SettingWidget::slotAutoStartChanged(int state)
{
	writeData(sender());
}

void SettingWidget::slotOpacityChanged()
{
	writeData(sender());
}

void SettingWidget::slotCurrentFontChanged(const QFont &font)
{
	writeData(sender());
}

void SettingWidget::slotBoldChanged(int state)
{
	writeData(sender());
}

void SettingWidget::slotDefaultActivated(const QString &link)
{
	Acc::instance()->getSettingModel()->revertDefault();
	emit Acc::instance()->sigSetMainShortcut("");
	readData();
	writeData();
}