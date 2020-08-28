#include "SettingWidget.h"
#include <QAction>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include "controller/Acc.h"
#include "view/MainWidget.h"
#include "common/LocalSearch.h"
#include "common/DarkStyle.h"
#include "common/Util.h"

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
    connect(ui.labelMyBlog, &QLabel::linkActivated, this, &SettingWidget::slotMyBlog);
	connect(ui.cbSearchEngine, SIGNAL(activated(QString)), this, SLOT(slotSearchEngineActivated(QString)));
	connect(ui.cbOpenUrlOn, &QCheckBox::clicked, [this](){ this->writeData(ui.cbOpenUrlOn); });
	connect(ui.cbSearchEngineOn, &QCheckBox::clicked, [this]() { this->writeData(ui.cbSearchEngineOn); });
    connect(ui.cbUseBing, &QCheckBox::clicked, [this]() { this->writeData(ui.cbUseBing); });
    connect(ui.pbEnvVarConfirm, &QPushButton::clicked, [this]() {
        Util::ImproveProcPriv();
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 1000, NULL);
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 1000, NULL);
    });

    for (int i = 0; i <= 7; i++) {
        ui.cbWallpaperIndex->addItem(QString::number(i));
    }
    connect(ui.cbWallpaperIndex, SIGNAL(activated(int)), this, SLOT(slotWallpaperIndex(int)));

	QStringList searchEngineList = QStringList() << tr("Baidu") << tr("Bing") << tr("Google");
	ui.cbSearchEngine->addItems(searchEngineList);

    // index tab
    ui.indexListWidget->setStyleSheet("QListWidget::item{ padding-left:2px;}");
    ui.indexListWidget->setAutoFillBackground(true);

    connect(ui.pbIndexAdd, &QPushButton::clicked, this, &SettingWidget::slotIndexAdd);
    connect(ui.pbIndexRemove, &QPushButton::clicked, this, &SettingWidget::slotIndexRemove);
    connect(ui.pbIndexUp, &QPushButton::clicked, this, &SettingWidget::slotIndexUp);
    connect(ui.pbIndexDown, &QPushButton::clicked, this, &SettingWidget::slotIndexDown);
    connect(Acc::instance(), &Acc::sigIndexResult, this, &SettingWidget::slotIndexResult);

	readData();
}

SettingWidget::~SettingWidget()
{
}

void SettingWidget::readData()
{
	SettingModel *settingModel = Acc::instance()->getSettingModel();
	ui.cbMaxResult->setCurrentText(QString::number(settingModel->maxResult()));
	ui.leHotkey->setText(settingModel->mainShortcutText());
	ui.cbAutoStart->setChecked(settingModel->autoStart());
    ui.hsOpacity->setValue(MAX_OPACITY - settingModel->mainOpacity());
	ui.fcbFont->setCurrentText(settingModel->fontFamily());
	ui.cbBold->setChecked(settingModel->isBold());
	ui.cbOpenUrlOn->setChecked(settingModel->enableOpenUrl());
	ui.cbSearchEngineOn->setChecked(settingModel->searchEngine().first);
	ui.cbSearchEngine->setCurrentText(settingModel->searchEngine().second);
    ui.cbUseBing->setChecked(settingModel->bindWallpaperUrl().first);
    ui.cbWallpaperIndex->setCurrentIndex(settingModel->bindWallpaperUrl().second);
    QStringList indexList = settingModel->getIndexList();
    foreach(const QString &index, indexList) {
        if (LocalSearcher::instance().isExit(Util::md5(index))) {
            addIndexItem(index);
        }
    }
    updateIndexStatus();
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
	if (!sender || ui.pbHotkeyConfirm == sender) {
		QString text = ui.leHotkey->text().trimmed();
        text.remove(" ");
        uint keyCode = Util::toKey(text);
        text = QKeySequence(keyCode).toString();
        if (text.isEmpty()) {
            QMessageBox::warning(nullptr, tr("Warning"), tr("Shortcut key Format error"));
        } else {
            emit Acc::instance()->sigSetMainShortcut(text);
        }
	}

	// 开机重启
	if (!sender || ui.cbAutoStart == sender) {
		bool isAutoStart = ui.cbAutoStart->isChecked();
		Acc::instance()->getSettingModel()->setAutoStart(isAutoStart);
	}

	// 透明度
	if (!sender || ui.hsOpacity == sender) {
        int opacity = MAX_OPACITY - ui.hsOpacity->value();
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
    // bing壁纸
    if (!sender || ui.cbUseBing == sender || ui.cbWallpaperIndex == sender) {
        Acc::instance()->getSettingModel()->setBindWallpaper(ui.cbUseBing->isChecked(), ui.cbWallpaperIndex->currentIndex());
    }
    // 设置索引
    if (!sender || ui.indexListWidget == sender) {
        QStringList indexList;
        for (int i = 0; i < ui.indexListWidget->count(); i++) {
            indexList.push_back(ui.indexListWidget->item(i)->text());
        }
        Acc::instance()->getSettingModel()->setIndexList(indexList);
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

void SettingWidget::slotMyBlog(const QString &link)
{
    Util::shellExecute(link);
}

void SettingWidget::slotSearchEngineActivated(const QString &text)
{
	writeData(sender());
}

void SettingWidget::slotWallpaperIndex(int index)
{
    writeData(sender());
    Acc::instance()->setBindWallpaper(index);
}

void SettingWidget::slotIndexAdd()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select index directory"));
    if (dir.isEmpty()) {
        return;
    }

    auto newItem = addIndexItem(dir);
    writeData(ui.indexListWidget);

    Acc::instance()->getLnkModel()->load(dir);
    updateIndexStatus();
}

void SettingWidget::slotIndexRemove()
{
    auto items = ui.indexListWidget->selectedItems();
    if (items.size() != 1) {
        return;
    }

    QString removeText = items[0]->text();
    if (Acc::instance()->getLnkModel()->removeSearcher(Util::md5(removeText))) {
        int delRow = ui.indexListWidget->row(items[0]);
        auto removeItem = ui.indexListWidget->takeItem(delRow);
        ui.indexListWidget->removeItemWidget(removeItem);
        writeData(ui.indexListWidget);
        updateIndexStatus();
    }    
}

void SettingWidget::slotIndexUp()
{
    auto items = ui.indexListWidget->selectedItems();
    if (items.size() != 1) {
        return;
    }

    int row = ui.indexListWidget->row(items[0]);
    if (row <= 0) {
        return;
    }

    auto text = items[0]->text();
    auto upItem = ui.indexListWidget->item(row - 1);
    items[0]->setText(upItem->text());
    upItem->setText(text);
    
    ui.indexListWidget->setCurrentItem(upItem);
    writeData(ui.indexListWidget);

    updateIndexStatus();
}

void SettingWidget::slotIndexDown()
{
    auto items = ui.indexListWidget->selectedItems();
    if (items.size() != 1) {
        return;
    }

    int row = ui.indexListWidget->row(items[0]);
    if (ui.indexListWidget->count() - 1 == row) {
        return;
    }

    auto text = items[0]->text();
    auto downItem = ui.indexListWidget->item(row + 1);
    items[0]->setText(downItem->text());
    downItem->setText(text);

    ui.indexListWidget->setCurrentItem(downItem);
    writeData(ui.indexListWidget);

    //Acc::instance()->getLnkModel()->sortSearcher();
    updateIndexStatus();
}

QListWidgetItem* SettingWidget::addIndexItem(const QString &name)
{
    for (int i = 0; i < ui.indexListWidget->count(); i++) {
        if (ui.indexListWidget->item(i)->text() == name) {
            return nullptr;
        }
    }

    auto item = new QListWidgetItem(QIcon(":/images/wait.png"), name);
    item->setSizeHint(QSize(item->sizeHint().width(), INDEX_ROW_HEIGHT));
    ui.indexListWidget->addItem(item);
    return item;
}

void SettingWidget::addIndexItemList(const QStringList &nameList)
{
    for (int i = 0; i < nameList.size(); i++) {
        addIndexItem(nameList[i]);
    }
    updateIndexStatus();
}

void SettingWidget::updateIndexStatus()
{
    for (int i = 0; i < ui.indexListWidget->count(); i++) {
        QString text = ui.indexListWidget->item(i)->text();
        if (LocalSearcher::instance().isExit(Util::md5(text))) {
            ui.indexListWidget->item(i)->setIcon(QIcon(":/images/ok.png"));
        } else {
            ui.indexListWidget->item(i)->setIcon(QIcon(":/images/wait.png"));
        }
    }
}

void SettingWidget::slotIndexResult(const QString &err, const QString &indexName)
{
    updateIndexStatus();
}
