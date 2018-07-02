#include "LnkListView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QScrollBar>
#include "component/ImageButton.h"
#include "common/Util.h"
#include "controller/Acc.h"

LnkListView::LnkListView(QWidget *parent)
    : QListView(parent), shieldBtn_(nullptr)
{
	this->setAlternatingRowColors(true);
	this->setMouseTracking(true); // 否则mouse over样式不起作用
	this->setSelectionMode(QAbstractItemView::SingleSelection);

	folderOpenBtn_ = new ImageButton(this);
    folderOpenBtn_->setImages(Util::img("folder_open"), Util::img("folder_open_hover"), Util::img("folder_open_hover"));
    connect(folderOpenBtn_, &QPushButton::clicked, this, &LnkListView::slotFolerOpen);

    if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
        shieldBtn_ = new ImageButton(this);
        shieldBtn_->setImages(Util::img("shield"), Util::img("shield_hover"), Util::img("shield_hover"));
        connect(shieldBtn_, &QPushButton::clicked, this, &LnkListView::slotShieldOpen);
    }

    setEnableButtonsVisible(false);
	connect(this, &QListView::clicked, this, &LnkListView::slotItemClicked);
	connect(this->verticalScrollBar(), &QScrollBar::sliderPressed, [this] { setEnableButtonsVisible(false); });
}

LnkListView::~LnkListView()
{
}

void LnkListView::selectNext()
{
	int nextRow = 0;
	QModelIndex index = currentIndex();
	if (index.isValid() && index.row() + 1 < this->model()->rowCount()) {
		nextRow = index.row() + 1;
	}
	setSelect(nextRow);
}

void LnkListView::selectPrev()
{
	int prevRow = this->model()->rowCount() - 1;
	QModelIndex index = currentIndex();
	if (index.isValid() && index.row() - 1 >= 0) {
		prevRow = index.row() - 1;
	}
	setSelect(prevRow);
}

void LnkListView::setSelect(int row)
{
	setEnableButtonsVisible(false);
	QModelIndex index = this->model()->index(row, 0);
	if (index.isValid()) {
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
		this->scrollTo(index);
	}
}

QModelIndex LnkListView::currentIndex()
{
	QModelIndexList indexList = this->selectionModel()->selectedRows();
	return indexList.size() > 0 ? indexList[0] : QModelIndex();
}

void LnkListView::openIndex(const QModelIndex &index)
{
	if (!index.isValid()) {
		return;
	}

	Acc::instance()->hideWidget(WidgetID::MAIN);
	QVariantMap vm = this->model()->data(index).toMap();
	QString path = vm["targetPath"].toString();
	if (!path.isEmpty() && Util::shellExecute(path)) {
		QString title = vm["lnkName"].toString();
		QString subtitle = vm["targetPath"].toString();
		Acc::instance()->getHitsModel()->increase(title, subtitle);
		Acc::instance()->getHitsModel()->unload();
	}
}

void LnkListView::mouseMoveEvent(QMouseEvent *e)
{
	QListView::mouseMoveEvent(e);
	QModelIndex index = this->indexAt(e->pos());
	if (index.isValid()) {
		setEnableButtonsVisible(true);
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
	}
	else {
		setEnableButtonsVisible(false);
	}
}

void LnkListView::wheelEvent(QWheelEvent *e)
{
	QListView::wheelEvent(e);
	QModelIndex index = this->indexAt(e->pos());
	if (index.isValid()) {
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
	}
}

void LnkListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QListView::selectionChanged(selected, deselected);
	QModelIndexList indexList = selected.indexes();
    if (indexList.size() > 0 && enableButtonsVisible()) {
		QModelIndex index = indexList[0];
		if (index.isValid()) {
			folderOpenBtn_->setProperty("index", index);
			folderOpenBtn_->show();
			QScrollBar *scrollbar = this->verticalScrollBar();
			int x = this->rect().width() - 40 - (scrollbar->isVisible() ? scrollbar->width() : 0);
			folderOpenBtn_->move(x, (index.row() - scrollbar->value()) * ROW_HEIGHT + 5);

            if (shieldBtn_) {
                shieldBtn_->setProperty("index", index);
                shieldBtn_->show();
                shieldBtn_->move(x - 30, (index.row() - scrollbar->value()) * ROW_HEIGHT + 5);
            }
		}
		else {
			folderOpenBtn_->hide();
            if (shieldBtn_) {
                shieldBtn_->hide();
            }
		}
	}
}

void LnkListView::setEnableButtonsVisible(bool visible)
{
	folderOpenBtn_->setProperty("enableVisible", visible);
	folderOpenBtn_->setVisible(visible);
    if (shieldBtn_) {
        shieldBtn_->setVisible(visible);
    }
}

bool LnkListView::enableButtonsVisible() const
{
    return folderOpenBtn_->property("enableVisible").toBool();
}

QString LnkListView::getPathFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        QString targetPath = this->model()->data(index).toMap()["targetPath"].toString();
        QFileInfo info(targetPath);
        if (info.exists()) {
            return targetPath;
        }
    }
    return "";
}

void LnkListView::slotFolerOpen()
{
    Util::locateFile(getPathFromIndex(this->sender()->property("index").toModelIndex()));
}

void LnkListView::slotShieldOpen()
{
    Util::shellExecute(getPathFromIndex(this->sender()->property("index").toModelIndex()), "runas");
}

void LnkListView::slotItemClicked(const QModelIndex &index)
{
	openIndex(index);
}
