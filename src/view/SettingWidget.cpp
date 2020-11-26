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

    ui.indexTableWidget->setHorizontalHeaderLabels(QStringList() << "Key" << "Path" << "Filter");
    ui.indexTableWidget->setColumnCount(2);
    ui.indexTableWidget->verticalHeader()->hide();
    ui.indexTableWidget->horizontalHeader()->hide();
    // 去掉虚线框
    ui.indexTableWidget->setShowGrid(false);
    // 单行选择
    ui.indexTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.indexTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // 禁止编辑
    ui.indexTableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    // 关闭水平垂直滚动条，使用自定义的悬浮滚动条（为了满足样式需求）
    ui.indexTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 鼠标跟踪
    ui.indexTableWidget->setMouseTracking(true);
    // 扩展最后一列
    //ui.indexTableWidget->horizontalHeader()->setStretchLastSection(true);

    ui.indexTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
    ui.indexTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui.indexTableWidget, &QTableWidget::itemSelectionChanged, this, &SettingWidget::slotIndexTableSelectChanged);

    // index tab
    //ui.indexListWidget->setStyleSheet("QListWidget::item{ padding-left:2px;}");
    //ui.indexListWidget->setAutoFillBackground(true);

    connect(ui.pbIndexOpen, &QPushButton::clicked, this, &SettingWidget::slotIndexOpen);
    connect(ui.pbIndexSave, &QPushButton::clicked, this, &SettingWidget::slotIndexSave);
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
    QList<IndexInfo> indexList = settingModel->getIndexList();
    foreach(const IndexInfo &index, indexList) {
        addIndexItem(index);
    }
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
    //if (!sender || ui.indexListWidget == sender) {
    //    QStringList indexList;
    //    for (int i = 0; i < ui.indexListWidget->count(); i++) {
    //        indexList.push_back(ui.indexListWidget->item(i)->text());
    //    }
    //    Acc::instance()->getSettingModel()->setIndexList(indexList);
    //}
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

void SettingWidget::slotIndexTableSelectChanged()
{
    IndexInfo info;
    QList<QTableWidgetItem*> items = ui.indexTableWidget->selectedItems();
    int row = getSelectRow();
    if (row >= 0) {
        info = Acc::instance()->getSettingModel()->getIndex(ui.indexTableWidget->item(row, 0)->text());
    }

    ui.indexKey->setText(info.key);
    ui.indexFilter->setText(info.filter);
    ui.indexPath->setText(info.path);
}

void SettingWidget::slotIndexOpen()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select index directory"));
    if (dir.isEmpty()) {
        return;
    }

    int nameIndex = dir.lastIndexOf("/");
    if (nameIndex < 0) {
        nameIndex = dir.lastIndexOf("\\");
    }
    if (nameIndex >= 0) {
        ui.indexKey->setText(dir.mid(nameIndex + 1).toLower());
    }
    ui.indexPath->setText(dir);
    ui.indexFilter->setText("*.*");
}

void SettingWidget::slotIndexSave()
{
    IndexInfo info;
    info.key = ui.indexKey->text().trimmed();
    info.path = ui.indexPath->text().trimmed();
    info.filter = ui.indexFilter->text().trimmed();
    if (info.isEmpty()) {
        return;
    }

    addIndexItem(info);
    updateIndexModel();
    QSharedPointer<ModelData> modelData = Acc::instance()->getLnkModel()->getModelData("lnk");
    QSharedPointer<LnkModelData> lnkModelData = modelData.staticCast<LnkModelData>();
    if (lnkModelData) {
        lnkModelData->load(info.key, info.path, info.filter);
    }
}

void SettingWidget::slotIndexRemove()
{
    int row = getSelectRow();
    if (row < 0) {
        return;
    }
    QString key = ui.indexTableWidget->item(row, 0)->text();
    ui.indexTableWidget->removeRow(row);
    updateIndexModel();
}

void SettingWidget::slotIndexUp()
{
    QList<IndexInfo> infoList = getCurrentIndexList();
    int row = getSelectRow();
    if (row > 0) {
        IndexInfo tmp = infoList[row];
        infoList[row] = infoList[row - 1];
        infoList[row - 1] = tmp;
        clearIndexTableAllRow();
        foreach(const IndexInfo &info, infoList) {
            addIndexItem(info);
        }
        ui.indexTableWidget->selectRow(row - 1);
        updateIndexModel();
    }
}

void SettingWidget::slotIndexDown()
{
    QList<IndexInfo> infoList = getCurrentIndexList();
    int row = getSelectRow();
    if (row >= 0 && row < infoList.size() - 1) {
        IndexInfo tmp = infoList[row];
        infoList[row] = infoList[row + 1];
        infoList[row + 1] = tmp;
        clearIndexTableAllRow();
        foreach(const IndexInfo &info, infoList) {
            addIndexItem(info);
        }
        ui.indexTableWidget->selectRow(row + 1);
        updateIndexModel();
    }
}

void SettingWidget::addIndexItem(const IndexInfo &info)
{
    int rowCount = ui.indexTableWidget->rowCount();
    for (int i = 0; i < rowCount; i++) {
        if (info.key == ui.indexTableWidget->item(i, 0)->text()) {
            ui.indexTableWidget->item(i, 0)->setData(Qt::UserRole + 1, info.toMap());
            ui.indexTableWidget->item(i, 1)->setText(info.path);
            return;
        }
    }

    int newRow = rowCount;
    ui.indexTableWidget->setRowCount(rowCount + 1);
    QTableWidgetItem *keyItem = new QTableWidgetItem(info.key);
    keyItem->setData(Qt::UserRole + 1, info.toMap());
    ui.indexTableWidget->setItem(newRow, 0, keyItem);
    ui.indexTableWidget->setItem(newRow, 1, new QTableWidgetItem(info.path));
}

void SettingWidget::updateIndexModel()
{
    Acc::instance()->getSettingModel()->setIndexList(getCurrentIndexList());
    Acc::instance()->getSettingModel()->sync();
    QSharedPointer<ModelData> modelData = Acc::instance()->getLnkModel()->getModelData("cmd");
    QSharedPointer<CmdModelData> cmdModelData = modelData.staticCast<CmdModelData>();
    if (cmdModelData) {
        cmdModelData->updateIndexDir();
    }
}

int SettingWidget::getSelectRow() const
{
    QList<QTableWidgetItem*> items = ui.indexTableWidget->selectedItems();
    if (items.isEmpty()) {
        return -1;
    }
    return items.first()->row();
}

void SettingWidget::clearIndexTableAllRow()
{
    for (int i = ui.indexTableWidget->rowCount() - 1; i >= 0; i--) {
        ui.indexTableWidget->removeRow(i);
    }
}

QList<IndexInfo> SettingWidget::getCurrentIndexList() const
{
    QList<IndexInfo> infoList;
    for (int i = 0; i < ui.indexTableWidget->rowCount(); i++) {
        IndexInfo info;
        info.fromMap(ui.indexTableWidget->item(i, 0)->data(Qt::UserRole + 1).toMap());
        if (!info.isEmpty()) {
            infoList.append(info);
        }
    }
    return infoList;
}

void SettingWidget::slotIndexResult(const QString &err, const QString &indexName)
{
    
}
