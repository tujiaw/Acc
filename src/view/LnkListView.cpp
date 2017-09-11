#include "LnkListView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QScrollBar>
#include "component/ImageButton.h"
#include "common/Util.h"

LnkListView::LnkListView(QWidget *parent)
	: QListView(parent)
{
	this->setAlternatingRowColors(true);
	this->setMouseTracking(true); // 否则mouse over样式不起作用
	this->setSelectionMode(QAbstractItemView::SingleSelection);

	folderOpenBtn_ = new ImageButton(this);
	folderOpenBtn_->setImages(Util::img("folder_open"), Util::img("folder_open_hover"), Util::img("folder_open_hover"));
	setFolderOpenBtnVisible(false);
	connect(folderOpenBtn_, &QPushButton::clicked, this, &LnkListView::slotFolerOpen);
	connect(this, &QListView::clicked, this, &LnkListView::slotItemClicked);
	connect(this->verticalScrollBar(), &QScrollBar::sliderPressed, [this] { setFolderOpenBtnVisible(false); });
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
	setFolderOpenBtnVisible(false);
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

	QVariantMap vm = this->model()->data(index).toMap();
	QString path = vm["targetPath"].toString();
	if (!path.isEmpty()) {
		Util::shellExecute(path);
	}
}

void LnkListView::mouseMoveEvent(QMouseEvent *e)
{
	QListView::mouseMoveEvent(e);
	QModelIndex index = this->indexAt(e->pos());
	if (index.isValid()) {
		setFolderOpenBtnVisible(true);
		this->selectionModel()->clear();
		this->selectionModel()->select(index, QItemSelectionModel::Select);
	}
	else {
		setFolderOpenBtnVisible(false);
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
	if (indexList.size() > 0 && folderOpenBtn_->property("enableVisible").toBool()) {
		QModelIndex index = indexList[0];
		if (index.isValid()) {
			folderOpenBtn_->setProperty("index", index);
			folderOpenBtn_->show();

			QScrollBar *scrollbar = this->verticalScrollBar();
			int x = this->rect().width() - 40 - (scrollbar->isVisible() ? scrollbar->width() : 0);
			folderOpenBtn_->move(x, (index.row() - scrollbar->value()) * ROW_HEIGHT + 5);
		}
		else {
			folderOpenBtn_->hide();
		}
	}
}

void LnkListView::setFolderOpenBtnVisible(bool visible)
{
	folderOpenBtn_->setProperty("enableVisible", visible);
	folderOpenBtn_->setVisible(visible);
}

void LnkListView::slotFolerOpen()
{
	QModelIndex index = this->sender()->property("index").toModelIndex();
	if (index.isValid()) {
		QString targetPath = this->model()->data(index).toMap()["targetPath"].toString();
		QFileInfo info(targetPath);
		if (info.exists()) {
			Util::locateFile(targetPath);
		}
	}
}

void LnkListView::slotItemClicked(const QModelIndex &index)
{
	openIndex(index);
}
