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
	
	// 这里应该用点击的信号，设置值得时候不应该触发槽函数
	connect(ui.cbMaxResult, SIGNAL(activated(QString)), this, SLOT(slotMaxResultChanged(QString)));
	connect(ui.pbHotkeyConfirm, &QPushButton::clicked, [this]() { this->writeData(ui.pbHotkeyConfirm); });
	connect(ui.cbAutoStart, &QCheckBox::clicked, [this]() { this->writeData(ui.cbAutoStart); });
	connect(ui.hsOpacity, &QSlider::sliderReleased, this, &SettingWidget::slotOpacityChanged);
	connect(ui.fcbFont, SIGNAL(activated(int)), this, SLOT(slotCurrentFontChanged(int)));
	connect(ui.cbBold, &QCheckBox::clicked, [this]() { this->writeData(ui.cbBold); });
	connect(ui.labelDefault, &QLabel::linkActivated, this, &SettingWidget::slotDefaultActivated);
	connect(ui.cbSearchEngine, SIGNAL(activated(QString)), this, SLOT(slotSearchEngineActivated(QString)));
	connect(ui.cbOpenUrlOn, &QCheckBox::clicked, [this](){ this->writeData(ui.cbOpenUrlOn); });
	connect(ui.cbSearchEngineOn, &QCheckBox::clicked, [this]() { this->writeData(ui.cbSearchEngineOn); });

	QStringList menuList = QStringList() << tr("Hot Key") << tr("Start") << tr("Shown");
	for (int i = 0; i < menuList.size(); i++) {
		ui.listWidget->addItem(menuList[i]);
	}

	QStringList searchEngineList = QStringList() << tr("Baidu") << tr("Bing") << tr("Google");
	ui.cbSearchEngine->addItems(searchEngineList);

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
	ui.fcbFont->setCurrentText(settingModel->fontFamily());
	ui.cbBold->setChecked(settingModel->isBold());
	ui.cbOpenUrlOn->setChecked(settingModel->enableOpenUrl());
	ui.cbSearchEngineOn->setChecked(settingModel->searchEngine().first);
	ui.cbSearchEngine->setCurrentText(settingModel->searchEngine().second);
}

void SettingWidget::writeData(QObject *sender)
{
	SettingModel *settingModel = Acc::instance()->getSettingModel();

	// 最大显示列个数
	if (!sender || ui.cbMaxResult == sender) {
		bool ok;
		int count = ui.cbMaxResult->currentText().toInt(&ok);
		if (ok) {
			settingModel->setMaxResult(count);
			emit Acc::instance()->sigClearResult();
		}
	}

	// 热键
	if (!sender || ui.leHotkey == sender) {
		QString text = ui.leHotkey->text().trimmed();
		text.remove(" ");
		emit Acc::instance()->sigSetMainShortcut(text);
	}

	// 开机重启
	if (!sender || ui.cbAutoStart == sender) {
		bool isAutoStart = ui.cbAutoStart->isChecked();
		Acc::instance()->getSettingModel()->setAutoStart(isAutoStart);
	}

	// 透明度
	if (!sender || ui.hsOpacity == sender) {
		int opacity = ui.hsOpacity->maximum() - ui.hsOpacity->value();
		Acc::instance()->setWindowOpacity(WidgetID::MAIN, opacity);
		Acc::instance()->getSettingModel()->setMainOpacity(opacity);
	}

	// 字体
	if (!sender || (ui.cbBold == sender || ui.fcbFont == sender)) {
		bool isBold = ui.cbBold->isChecked();
		QString family = ui.fcbFont->currentText();
		CDarkStyle::setFontFamily(family, isBold);
		CDarkStyle::assign();
		Acc::instance()->getSettingModel()->setFontFamily(family, isBold);
	}

	if (!sender || (ui.cbOpenUrlOn == sender)) {
		Acc::instance()->getSettingModel()->setEnableOpenUrl(ui.cbOpenUrlOn->isChecked());
	}
	// 搜索引擎
	if (!sender || (ui.cbSearchEngineOn == sender || ui.cbSearchEngine == sender)) {
		Acc::instance()->getSettingModel()->setSearchEngine(ui.cbSearchEngineOn->isChecked(), ui.cbSearchEngine->currentText());
	}
}

void SettingWidget::slotMaxResultChanged(const QString &text)
{
	writeData(sender());
}

void SettingWidget::slotOpacityChanged()
{
	writeData(sender());
}

void SettingWidget::slotCurrentFontChanged(int index)
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

void SettingWidget::slotSearchEngineActivated(const QString &text)
{
	writeData(sender());
}